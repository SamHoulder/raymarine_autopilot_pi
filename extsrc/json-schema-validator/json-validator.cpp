/*
 * JSON schema validator for JSON for modern C++
 *
 * Copyright (c) 2016-2019 Patrick Boettcher <p@yai.se>.
 *
 * SPDX-License-Identifier: MIT
 *
 */
#include <json-schema.hpp>

#include <memory>
#include <set>
#ifdef __MSVC__
#include <sstream>
#endif

#ifdef __OCPN__ANDROID__
#include <sstream>
#endif

using nlohmann::json;
using nlohmann::json_uri;
using nlohmann::json_schema::root_schema;
using namespace nlohmann::json_schema;

#ifdef JSON_SCHEMA_BOOST_REGEX
#	include <boost/regex.hpp>
#	define REGEX_NAMESPACE boost
#elif defined(JSON_SCHEMA_NO_REGEX)
#	define NO_STD_REGEX
#else
#	include <regex>
#	define REGEX_NAMESPACE std
#endif

namespace
{

class schema
{
protected:
	root_schema *root_;

public:
	schema(root_schema *root)
	    : root_(root) {}

	virtual void validate(const json &instance, basic_error_handler &e) const = 0;

	static std::shared_ptr<schema> make(json &schema,
	                                    root_schema *root,
	                                    const std::vector<std::string> &key,
	                                    std::vector<nlohmann::json_uri> uris);
};

class logical_not : public schema
{
	std::shared_ptr<schema> subschema_;

	void validate(const json &instance, basic_error_handler &e) const final
	{
		basic_error_handler err;
		subschema_->validate(instance, err);

		if (!err)
			e.error("", instance, "instance is valid, whereas it should NOT be as required by schema");
	}

public:
	logical_not(json &sch,
	            root_schema *root,
	            const std::vector<nlohmann::json_uri> &uris)
	    : schema(root)
	{
		subschema_ = schema::make(sch, root, {"not"}, uris);
	}
};

enum logical_combination_types {
	allOf,
	anyOf,
	oneOf
};

template <enum logical_combination_types combine_logic>
class logical_combination : public schema
{
	std::vector<std::shared_ptr<schema>> subschemata_;

	void validate(const json &instance, basic_error_handler &e) const final
	{
		size_t count = 0;

		for (auto &s : subschemata_) {
			basic_error_handler err;
			s->validate(instance, err);

			if (err) {
				//sub_schema_err << "  one schema failed because: " << e.what() << "\n";
				if (combine_logic == allOf) {
					e.error("", instance, "at least one schema has failed, but ALLOF them are required to validate.");
					return;
				}
			} else
				count++;

			if (combine_logic == oneOf && count > 1) {
				e.error("", instance, "more than one schema has succeeded, but only ONEOF them is required to validate.");
				return;
			}
			if (combine_logic == anyOf && count == 1)
				return;
		}

		if ((combine_logic == anyOf || combine_logic == oneOf) && count == 0)
			e.error("", instance, "no validation has succeeded but ANYOF/ONEOF them is required to validate.");
	}

public:
	logical_combination(json &sch,
	                    root_schema *root,
	                    const std::vector<nlohmann::json_uri> &uris)
	    : schema(root)
	{
		size_t c = 0;
		std::string key;
		switch (combine_logic) {
		case allOf:
			key = "allOf";
			break;
		case oneOf:
			key = "oneOf";
			break;
		case anyOf:
			key = "anyOf";
			break;
		}

		for (auto &subschema : sch)
			subschemata_.push_back(schema::make(subschema, root, {key, std::to_string(c++)}, uris));
	}
};

class type_schema : public schema
{
	std::vector<std::shared_ptr<schema>> type_;
	std::pair<bool, json> enum_, const_;
	std::vector<std::shared_ptr<schema>> logic_;

	static std::shared_ptr<schema> make(json &schema,
	                                    json::value_t type,
	                                    root_schema *,
	                                    const std::vector<nlohmann::json_uri> &,
	                                    std::set<std::string> &);

	std::shared_ptr<schema> if_, then_, else_;

	void validate(const json &instance, basic_error_handler &e) const override final
	{
		// depending on the type of instance run the type specific validator - if present
		auto type = type_[(uint8_t) instance.type()];

		if (type)
			type->validate(instance, e);
		else
			e.error("", instance, "unexpected instance type");

		if (enum_.first) {
			bool seen_in_enum = false;
			for (auto &e : enum_.second)
				if (instance == e) {
					seen_in_enum = true;
					break;
				}

			if (!seen_in_enum)
				e.error("", instance, "instance not found in required enum");
		}

		if (const_.first &&
		    const_.second != instance)
			e.error("", instance, "instance not const");

		for (auto l : logic_)
			l->validate(instance, e);

		if (if_) {
			basic_error_handler err;

			if_->validate(instance, err);
			if (!err) {
				if (then_)
					then_->validate(instance, e);
			} else {
				if (else_)
					else_->validate(instance, e);
			}
		}
	}

public:
	type_schema(json &sch,
	            root_schema *root,
	            const std::vector<nlohmann::json_uri> &uris)
	    : schema(root), type_((uint8_t) json::value_t::discarded + 1)
	{
		// association between JSON-schema-type and NLohmann-types
		static const std::vector<std::pair<std::string, json::value_t>> schema_types = {
		    {"null", json::value_t::null},
		    {"object", json::value_t::object},
		    {"array", json::value_t::array},
		    {"string", json::value_t::string},
		    {"boolean", json::value_t::boolean},
		    {"integer", json::value_t::number_integer},
		    {"integer", json::value_t::number_unsigned},
		    {"number", json::value_t::number_float},
		};

		std::set<std::string> known_keywords;

		auto attr = sch.find("type");
		if (attr == sch.end()) // no type field means all sub-types possible
			for (auto &t : schema_types)
				type_[(uint8_t) t.second] = type_schema::make(sch, t.second, root, uris, known_keywords);
		else {
			switch (attr.value().type()) { // "type": "type"

			case json::value_t::string: {
				auto schema_type = attr.value().get<std::string>();
				for (auto &t : schema_types)
					if (t.first == schema_type)
						type_[(uint8_t) t.second] = type_schema::make(sch, t.second, root, uris, known_keywords);
			} break;

			case json::value_t::array: // "type": ["type1", "type2"]
				for (auto &schema_type : attr.value())
					for (auto &t : schema_types)
						if (t.first == schema_type)
							type_[(uint8_t) t.second] = type_schema::make(sch, t.second, root, uris, known_keywords);
				break;

			default:
				break;
			}

			sch.erase(attr);
		}

		for (auto &key : known_keywords)
			sch.erase(key);

		// with nlohmann::json floats can be seen as unsigned or integer - reuse the number-validator for
		// integer values as well, if they have not been specified
		if (type_[(uint8_t) json::value_t::number_float] && !type_[(uint8_t) json::value_t::number_integer])
			type_[(uint8_t) json::value_t::number_integer] =
			    type_[(uint8_t) json::value_t::number_unsigned] =
			        type_[(uint8_t) json::value_t::number_float];

		attr = sch.find("enum");
		if (attr != sch.end()) {
			enum_ = {true, attr.value()};
			sch.erase(attr);
		}

		attr = sch.find("const");
		if (attr != sch.end()) {
			const_ = {true, attr.value()};
			sch.erase(attr);
		}

		attr = sch.find("not");
		if (attr != sch.end()) {
			logic_.push_back(std::make_shared<logical_not>(attr.value(), root, uris));
			sch.erase(attr);
		}

		attr = sch.find("allOf");
		if (attr != sch.end()) {
			logic_.push_back(std::make_shared<logical_combination<allOf>>(attr.value(), root, uris));
			sch.erase(attr);
		}

		attr = sch.find("anyOf");
		if (attr != sch.end()) {
			logic_.push_back(std::make_shared<logical_combination<anyOf>>(attr.value(), root, uris));
			sch.erase(attr);
		}

		attr = sch.find("oneOf");
		if (attr != sch.end()) {
			logic_.push_back(std::make_shared<logical_combination<oneOf>>(attr.value(), root, uris));
			sch.erase(attr);
		}

		attr = sch.find("if");
		if (attr != sch.end()) {
			auto attr_then = sch.find("then");
			auto attr_else = sch.find("else");

			if (attr_then != sch.end() || attr_else != sch.end()) {
				if_ = schema::make(attr.value(), root, {"if"}, uris);

				if (attr_then != sch.end()) {
					then_ = schema::make(attr_then.value(), root, {"then"}, uris);
					sch.erase(attr_then);
				}

				if (attr_else != sch.end()) {
					else_ = schema::make(attr_else.value(), root, {"else"}, uris);
					sch.erase(attr_else);
				}
			}
			sch.erase(attr);
		}
	}
};

class string : public schema
{
	std::pair<bool, size_t> maxLength_{false, 0};
	std::pair<bool, size_t> minLength_{false, 0};

#ifndef NO_STD_REGEX
	std::pair<bool, REGEX_NAMESPACE::regex> pattern_{false, REGEX_NAMESPACE::regex()};
	std::string patternString_;
#endif

	std::pair<bool, std::string> format_;
	std::function<void(const std::string &, const std::string &)> format_check_ = nullptr;

	std::size_t utf8_length(const std::string &s) const
	{
		size_t len = 0;
		for (const unsigned char &c : s)
			if ((c & 0xc0) != 0x80)
				len++;
		return len;
	}

	void validate(const json &instance, basic_error_handler &e) const override
	{
		if (minLength_.first) {
			if (utf8_length(instance) < minLength_.second) {
				std::ostringstream s;
				s << "'" << instance << "' is too short as per minLength (" << minLength_.second << ")";
				e.error("", instance, s.str());
			}
		}

		if (maxLength_.first) {
			if (utf8_length(instance) > maxLength_.second) {
				std::ostringstream s;
				s << "'" << instance << "' is too long as per maxLength (" << maxLength_.second << ")";
				e.error("", instance, s.str());
			}
		}

#ifndef NO_STD_REGEX
		if (pattern_.first &&
		    !REGEX_NAMESPACE::regex_search(instance.get<std::string>(), pattern_.second))
			e.error("", instance, instance.get<std::string>() + " does not match regex pattern: " + patternString_);
#endif

		if (format_.first) {
			if (format_check_ == nullptr)
				e.error("", instance, std::string("A format checker was not provided but a format-attribute for this string is present. ") + " cannot be validated for " + format_.second);
			else
				format_check_(format_.second, instance);
		}
	}

public:
	string(json &sch, root_schema *root)
	    : schema(root)
	{
		auto attr = sch.find("maxLength");
		if (attr != sch.end()) {
			maxLength_ = {true, attr.value()};
			sch.erase(attr);
		}

		attr = sch.find("minLength");
		if (attr != sch.end()) {
			minLength_ = {true, attr.value()};
			sch.erase(attr);
		}

#ifndef NO_STD_REGEX
		attr = sch.find("pattern");
		if (attr != sch.end()) {
			patternString_ = attr.value().get<std::string>();
			pattern_ = {true, REGEX_NAMESPACE::regex(attr.value().get<std::string>(),
			                                         REGEX_NAMESPACE::regex::ECMAScript)};
			sch.erase(attr);
		}
#endif

		attr = sch.find("format");
		if (attr != sch.end()) {
			format_ = {true, attr.value()};
			sch.erase(attr);
		}
	}
};

template <typename T>
class numeric : public schema
{
	std::pair<bool, T> maximum_{false, 0};
	std::pair<bool, T> minimum_{false, 0};

	bool exclusiveMaximum_ = false;
	bool exclusiveMinimum_ = false;

	std::pair<bool, json::number_float_t> multipleOf_{false, 0};

	// multipleOf - if the rest of the division is 0 -> OK
	bool violates_multiple_of(json::number_float_t x) const
	{
		json::number_integer_t n = static_cast<json::number_integer_t>(x / multipleOf_.second);
		double res = (x - n * multipleOf_.second);
		return fabs(res) > std::numeric_limits<json::number_float_t>::epsilon();
	}

	void validate(const json &instance, basic_error_handler &e) const override
	{
		T value = instance; // conversion of json to value_type

		if (multipleOf_.first && value != 0) // zero is multiple of everything
			if (violates_multiple_of(value))
				e.error("", instance, "is not a multiple of " + std::to_string(multipleOf_.second));

		if (maximum_.first)
			if ((exclusiveMaximum_ && value >= maximum_.second) ||
			    value > maximum_.second)
				e.error("", instance, "exceeds maximum of " + std::to_string(maximum_.second));

		if (minimum_.first)
			if ((exclusiveMinimum_ && value <= minimum_.second) ||
			    value < minimum_.second)
				e.error("", instance, "is below minimum of " + std::to_string(minimum_.second));
	}

public:
	numeric(const json &sch, root_schema *root, std::set<std::string> &kw)
	    : schema(root)
	{
		auto attr = sch.find("maximum");
		if (attr != sch.end()) {
			maximum_ = {true, attr.value()};
			kw.insert("maximum");
		}

		attr = sch.find("minimum");
		if (attr != sch.end()) {
			minimum_ = {true, attr.value()};
			kw.insert("minimum");
		}

		attr = sch.find("exclusiveMaximum");
		if (attr != sch.end()) {
			exclusiveMaximum_ = true;
			maximum_ = {true, attr.value()};
			kw.insert("exclusiveMaximum");
		}

		attr = sch.find("exclusiveMinimum");
		if (attr != sch.end()) {
			minimum_ = {true, attr.value()};
			exclusiveMinimum_ = true;
			kw.insert("exclusiveMinimum");
		}

		attr = sch.find("multipleOf");
		if (attr != sch.end()) {
			multipleOf_ = {true, attr.value()};
			kw.insert("multipleOf");
		}
	}
};

class null : public schema
{
	void validate(const json &instance, basic_error_handler &e) const override
	{
		if (!instance.is_null())
			e.error("", instance, "expected to be null");
	}

public:
	null(json &, root_schema *root)
	    : schema(root) {}
};

class boolean_type : public schema
{
	void validate(const json &, basic_error_handler &) const override {}

public:
	boolean_type(json &, root_schema *root)
	    : schema(root) {}
};

class boolean : public schema
{
	bool true_;
	void validate(const json &instance, basic_error_handler &e) const override
	{
		if (!true_) { // false schema
			// empty array
			//switch (instance.type()) {
			//case json::value_t::array:
			//	if (instance.size() != 0) // valid false-schema
			//		e.error("", instance, "false-schema required empty array");
			//	return;
			//}

			e.error("", instance, "instance invalid as par false-schema");
		}
	}

public:
	boolean(json &sch, root_schema *root)
	    : schema(root), true_(sch) {}
};

class required : public schema
{
	const std::vector<std::string> required_;

	void validate(const json &instance, basic_error_handler &e) const override final
	{
		for (auto &r : required_)
			if (instance.find(r) == instance.end())
				e.error("", instance, "required property '" + r + "' not found in object as a dependency");
	}

public:
	required(const std::vector<std::string> &r, root_schema *root)
	    : schema(root), required_(r) {}
};

class object : public schema
{
	std::pair<bool, size_t> maxProperties_{false, 0};
	std::pair<bool, size_t> minProperties_{false, 0};
	std::vector<std::string> required_;

	std::map<std::string, std::shared_ptr<schema>> properties_;
#ifndef NO_STD_REGEX
	std::vector<std::pair<REGEX_NAMESPACE::regex, std::shared_ptr<schema>>> patternProperties_;
#endif
	std::shared_ptr<schema> additionalProperties_;

	std::map<std::string, std::shared_ptr<schema>> dependencies_;

	std::shared_ptr<schema> propertyNames_;

	void validate(const json &instance, basic_error_handler &e) const override
	{
		if (maxProperties_.first && instance.size() > maxProperties_.second)
			e.error("", instance, "too many properties.");

		if (minProperties_.first && instance.size() < minProperties_.second)
			e.error("", instance, "too few properties.");

		for (auto &r : required_)
			if (instance.find(r) == instance.end())
				e.error("", instance, "required property '" + r + "' not found in object '");

		// for each property in instance
		for (auto &p : instance.items()) {
			if (propertyNames_)
				propertyNames_->validate(p.key(), e);

			bool a_prop_or_pattern_matched = false;
			auto schema_p = properties_.find(p.key());
			// check if it is in "properties"
			if (schema_p != properties_.end()) {
				a_prop_or_pattern_matched = true;
				schema_p->second->validate(p.value(), e);
			}

			// check all matching patternProperties
			for (auto &schema_pp : patternProperties_)
				if (REGEX_NAMESPACE::regex_search(p.key(), schema_pp.first)) {
					a_prop_or_pattern_matched = true;
					schema_pp.second->validate(p.value(), e);
				}
			// check additionalProperties as a last resort
			if (!a_prop_or_pattern_matched && additionalProperties_)
				additionalProperties_->validate(p.value(), e);
		}

		for (auto &dep : dependencies_) {
			auto prop = instance.find(dep.first);
			if (prop != instance.end())            // if dependency-property is present in instance
				dep.second->validate(instance, e); // validate
		}
	}

public:
	object(json &sch,
	       root_schema *root,
	       const std::vector<nlohmann::json_uri> &uris)
	    : schema(root)
	{
		auto attr = sch.find("maxProperties");
		if (attr != sch.end()) {
			maxProperties_ = {true, attr.value()};
			sch.erase(attr);
		}

		attr = sch.find("minProperties");
		if (attr != sch.end()) {
			minProperties_ = {true, attr.value()};
			sch.erase(attr);
		}

		attr = sch.find("required");
		if (attr != sch.end()) {
			required_ = attr.value().get<std::vector<std::string>>();
			sch.erase(attr);
		}

		attr = sch.find("properties");
		if (attr != sch.end()) {
			for (auto prop : attr.value().items())
				properties_.insert(
				    std::make_pair(
				        prop.key(),
				        schema::make(prop.value(), root, {"properties", prop.key()}, uris)));
			sch.erase(attr);
		}

#ifndef NO_STD_REGEX
		attr = sch.find("patternProperties");
		if (attr != sch.end()) {
			for (auto prop : attr.value().items())
				patternProperties_.push_back(
				    std::make_pair(
				        REGEX_NAMESPACE::regex(prop.key(), REGEX_NAMESPACE::regex::ECMAScript),
				        schema::make(prop.value(), root, {prop.key()}, uris)));
			sch.erase(attr);
		}
#endif

		attr = sch.find("additionalProperties");
		if (attr != sch.end()) {
			additionalProperties_ = schema::make(attr.value(), root, {"additionalProperties"}, uris);
			sch.erase(attr);
		}

		attr = sch.find("dependencies");
		if (attr != sch.end()) {
			for (auto &dep : attr.value().items())
				switch (dep.value().type()) {
				case json::value_t::array:
					dependencies_.emplace(dep.key(),
					                      std::make_shared<required>(
					                          dep.value().get<std::vector<std::string>>(), root));
					break;

				default:
					dependencies_.emplace(dep.key(),
					                      schema::make(dep.value(), root, {"dependencies", dep.key()}, uris));
					break;
				}
			sch.erase(attr);
		}

		attr = sch.find("propertyNames");
		if (attr != sch.end()) {
			propertyNames_ = schema::make(attr.value(), root, {"propertyNames"}, uris);
			sch.erase(attr);
		}
	}
};

class array : public schema
{
	std::pair<bool, size_t> maxItems_{false, 0};
	std::pair<bool, size_t> minItems_{false, 0};
	bool uniqueItems_ = false;

	std::shared_ptr<schema> items_schema_;

	std::vector<std::shared_ptr<schema>> items_;
	std::shared_ptr<schema> additionalItems_;

	std::shared_ptr<schema> contains_;

	void validate(const json &instance, basic_error_handler &e) const override
	{
		if (maxItems_.first && instance.size() > maxItems_.second)
			e.error("", instance, "has too many items.");

		if (minItems_.first && instance.size() < minItems_.second)
			e.error("", instance, "has too few items.");

		if (uniqueItems_) {
			for (auto it = instance.cbegin(); it != instance.cend(); ++it) {
				auto v = std::find(it + 1, instance.end(), *it);
				if (v != instance.end())
					e.error("", instance, "items have to be unique for this array.");
			}
		}

		if (items_schema_)
			for (auto &i : instance)
				items_schema_->validate(i, e);
		else {
			auto item = items_.cbegin();
			for (auto &i : instance) {
				std::shared_ptr<schema> item_validator;
				if (item == items_.cend())
					item_validator = additionalItems_;
				else {
					item_validator = *item;
					item++;
				}

				if (!item_validator)
					break;

				item_validator->validate(i, e);
			}
		}

		if (contains_) {
			bool contained = false;
			for (auto &item : instance) {
				basic_error_handler local_e;
				contains_->validate(item, local_e);
				if (!local_e) {
					contained = true;
					break;
				}
			}
			if (!contained)
				e.error("", instance, "array does not contain required element as per 'contains'");
		}
	}

public:
	array(json &sch, root_schema *root, const std::vector<nlohmann::json_uri> &uris)
	    : schema(root)
	{
		auto attr = sch.find("maxItems");
		if (attr != sch.end()) {
			maxItems_ = {true, attr.value()};
			sch.erase(attr);
		}

		attr = sch.find("minItems");
		if (attr != sch.end()) {
			minItems_ = {true, attr.value()};
			sch.erase(attr);
		}

		attr = sch.find("uniqueItems");
		if (attr != sch.end()) {
			uniqueItems_ = attr.value();
			sch.erase(attr);
		}

		attr = sch.find("items");
		if (attr != sch.end()) {

			if (attr.value().type() == json::value_t::array) {
				size_t c = 0;
				for (auto &subsch : attr.value())
					items_.push_back(schema::make(subsch, root, {"items", std::to_string(c++)}, uris));

				auto attr_add = sch.find("additionalItems");
				if (attr_add != sch.end()) {
					additionalItems_ = schema::make(attr_add.value(), root, {"additionalItems"}, uris);
					sch.erase(attr_add);
				}

			} else if (attr.value().type() == json::value_t::object ||
			           attr.value().type() == json::value_t::boolean)
				items_schema_ = schema::make(attr.value(), root, {"items"}, uris);

			sch.erase(attr);
		}

		attr = sch.find("contains");
		if (attr != sch.end()) {
			contains_ = schema::make(attr.value(), root, {"contains"}, uris);
			sch.erase(attr);
		}
	}
};

class schema_ref : public schema
{
	const std::string id_;
	std::shared_ptr<schema> target_;

	void validate(const json &instance, basic_error_handler &e) const final
	{
		if (target_)
			target_->validate(instance, e);
		else
			e.error("", instance, "unresolved schema-reference " + id_);
	}

public:
	schema_ref(const std::string &id, root_schema *root)
	    : schema(root), id_(id) {}

	const std::string &id() const { return id_; }
	void set_target(std::shared_ptr<schema> target) { target_ = target; }
};

std::shared_ptr<schema> type_schema::make(json &schema,
                                          json::value_t type,
                                          root_schema *root,
                                          const std::vector<nlohmann::json_uri> &uris,
                                          std::set<std::string> &kw)
{
	switch (type) {
	case json::value_t::null:
		return std::make_shared<null>(schema, root);
	case json::value_t::number_unsigned:
		return std::make_shared<numeric<json::number_unsigned_t>>(schema, root, kw);
	case json::value_t::number_integer:
		return std::make_shared<numeric<json::number_integer_t>>(schema, root, kw);
	case json::value_t::number_float:
		return std::make_shared<numeric<json::number_float_t>>(schema, root, kw);
	case json::value_t::string:
		return std::make_shared<string>(schema, root);
	case json::value_t::boolean:
		return std::make_shared<boolean_type>(schema, root);
	case json::value_t::object:
		return std::make_shared<object>(schema, root, uris);
	case json::value_t::array:
		return std::make_shared<array>(schema, root, uris);

	case json::value_t::discarded: // not a real type - silence please
		break;
	}
	return nullptr;
}
} // namespace

namespace nlohmann
{
namespace json_schema
{

class root_schema : public schema
{
	std::function<void(const json_uri &, json &)> loader_;
	std::function<void(const std::string &, const std::string &)> format_;

	std::shared_ptr<schema> root_;

	struct schema_file {
		std::map<nlohmann::json::json_pointer, std::shared_ptr<schema>> schemas;
		std::map<nlohmann::json::json_pointer, std::shared_ptr<schema_ref>> unresolved; // contains all unresolved references from any other file seen during parsing
		json unknown_keywords;
	};

	// location as key
	std::map<std::string, schema_file> files_;

	schema_file &get_or_create_file(const std::string &loc)
	{
		auto file = files_.lower_bound(loc);
		if (file != files_.end() && !(files_.key_comp()(loc, file->first)))
			return file->second;
		else
			return files_.insert(file, {loc, {}})->second;
	}

public:
	root_schema(std::function<void(const json_uri &, json &)> loader,
	            std::function<void(const std::string &, const std::string &)> format)
	    : schema(this), loader_(loader), format_(format) {}

	void insert(const json_uri &uri, const std::shared_ptr<schema> &s)
	{
		// std::cout << "adding schema '" << uri << "' '" << uri.location() << "'\n";

		auto &file = get_or_create_file(uri.location());
		auto schema = file.schemas.lower_bound(uri.pointer());
		if (schema != file.schemas.end() && !(file.schemas.key_comp()(uri.pointer(), schema->first))) {
			throw std::invalid_argument("schema with " + uri.to_string() + " already inserted\n");
			return;
		}

		file.schemas.insert({uri.pointer(), s});

		// was someone referencing this newly inserted schema?
		auto unresolved = file.unresolved.find(uri.pointer());
		// std::cout << "resolving schemas looking for '" << uri.pointer() << "' in " << uri.location() << "\n";
		if (unresolved != file.unresolved.end()) {
			// std::cout << " --> resolved!!\n";
			unresolved->second->set_target(s);
			file.unresolved.erase(unresolved);
		}
	}

	void insert_unknown_keyword(const json_uri &uri, const std::string &key, json &value)
	{
		auto &file = get_or_create_file(uri.location());
		auto new_uri = uri.append(key);
		auto pointer = new_uri.pointer();

		// std::cout << "inserting unknown " << new_uri << " '" << pointer << "'\n";

		// is there a reference looking for this unknown-keyword, which is thus no longer a unknown keyword but a schema
		auto unresolved = file.unresolved.find(pointer);
		if (unresolved != file.unresolved.end())
			schema::make(value, this, {}, {{new_uri}});
		else // no, nothing ref'd it
			file.unknown_keywords[pointer] = value;
	}

	std::shared_ptr<schema> get_or_create_ref(const json_uri &uri)
	{
		auto &file = get_or_create_file(uri.location());

		// existing schema
		auto schema = file.schemas.find(uri.pointer());
		if (schema != file.schemas.end())
			return schema->second;

		// referencing an unknown keyword, turn it into schema
		try {
			auto &subschema = file.unknown_keywords.at(uri.pointer());
			auto s = schema::make(subschema, this, {}, {{uri}});
			file.unknown_keywords.erase(uri.pointer());
			return s;
		} catch (...) {
		}

		// get or create a schema_ref
		// std::cout << "using or creating a reference to " << uri << "\n";
		auto r = file.unresolved.lower_bound(uri.pointer());
		if (r != file.unresolved.end() && !(file.unresolved.key_comp()(uri.pointer(), r->first))) {
			// std::cout << " --> using existing ref\n";
			return r->second;
		} else {
			// std::cout << " --> creating a new ref\n";
			return file.unresolved.insert(r,
			                              {uri.pointer(), std::make_shared<schema_ref>(uri.to_string(), this)})
			    ->second;
		}
	}

	void set_root_schema(json schema)
	{
		root_ = schema::make(schema, this, {}, {{"#"}});

		// load all files which have not yet been loaded
		do {
			bool new_schema_loaded = false;

			// files_ is modified during parsing, iterators are invalidated
			std::vector<std::string> locations;
			for (auto &file : files_)
				locations.push_back(file.first);

			for (auto &loc : locations) {
				if (files_[loc].schemas.size() == 0) { // nothing has been loaded for this file
					if (loader_) {
						json sch;

						loader_(loc, sch);

						schema::make(sch, this, {}, {{loc}});
						new_schema_loaded = true;
					} else {
						throw std::invalid_argument("external schema reference '" + loc + "' needs loading, but no loader callback given\n");
					}
				}
			}

			if (!new_schema_loaded) // if no new schema loaded, no need to try again
				break;
		} while (1);
	}

	void validate(const json &instance, basic_error_handler &e) const final
	{
		if (root_)
			root_->validate(instance, e);
		else
			e.error("", "", "no root schema has yet been set for validating an instance.");
	}
};

} // namespace json_schema
} // namespace nlohmann

namespace
{

std::shared_ptr<schema> schema::make(json &schema,
                                     root_schema *root,
                                     const std::vector<std::string> &keys,
                                     std::vector<nlohmann::json_uri> uris)
{
	// append to all URIs the keys for this sub-schema
	for (auto &key : keys)
		for (auto &uri : uris)
			uri = uri.append(json_uri::escape(key));

	std::shared_ptr<::schema> sch;

	// boolean schema
	if (schema.type() == json::value_t::boolean)
		sch = std::make_shared<boolean>(schema, root);
	else if (schema.type() == json::value_t::object) {

		auto attr = schema.find("$id"); // if $id is present, this schema can be referenced by this ID
		                                // as an additional URI
		if (attr != schema.end()) {
			if (std::find(uris.begin(),
			              uris.end(),
			              attr.value().get<std::string>()) == uris.end())
				uris.push_back(uris.back().derive(attr.value())); // so add it to the list if it is not there already
			schema.erase(attr);
		}

		attr = schema.find("definitions");
		if (attr != schema.end()) {
			for (auto &def : attr.value().items())
				schema::make(def.value(), root, {"definitions", def.key()}, uris);
			schema.erase(attr);
		}

		attr = schema.find("$ref");
		if (attr != schema.end()) { // this schema is a reference
			// the last one on the uri-stack is the last id seen before coming here,
			// so this is the origial URI for this reference, the $ref-value has thus be resolved from it
			auto id = uris.back().derive(attr.value());
			sch = root->get_or_create_ref(id);
			schema.erase(attr);
		} else {
			sch = std::make_shared<type_schema>(schema, root, uris);
		}

		schema.erase("$schema");
		schema.erase("default");
		schema.erase("title");
		schema.erase("description");
	} else {
		return nullptr; // TODO error/throw? when schema is invalid
	}

	for (auto &uri : uris) { // for all URI reference this schema
		root->insert(uri, sch);

		if (schema.type() == json::value_t::object)
			for (auto &u : schema.items())
				root->insert_unknown_keyword(uri, u.key(), u.value());
	}
	return sch;
}

class throwing_error_handler : public basic_error_handler
{
	void error(const std::string &path, const json &instance, const std::string &message)
	{
		throw std::invalid_argument(std::string("At ") + path + " of " + instance.dump() + " - " + message + "\n");
	}
};

} // namespace

namespace nlohmann
{
namespace json_schema
{

json_validator::json_validator(std::function<void(const json_uri &, json &)> loader,
                               std::function<void(const std::string &, const std::string &)> format)
    : root_(std::unique_ptr<root_schema>(new root_schema(loader, format)))
{
}

json_validator::~json_validator()
{
}

void json_validator::set_root_schema(const json &schema)
{
	root_->set_root_schema(schema);
}

void json_validator::validate(const json &instance)
{
	throwing_error_handler err;
	validate(instance, err);
}

void json_validator::validate(const json &instance, basic_error_handler &err)
{
	root_->validate(instance, err);
}

} // namespace json_schema
} // namespace nlohmann
