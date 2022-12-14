#ifndef RELATION_HPP
#define RELATION_HPP

#include "osmobject.hpp"
#include "cgimap/types.hpp"
#include "cgimap/util.hpp"

#include <optional>
#include <boost/algorithm/string/predicate.hpp>


namespace api06 {

class RelationMember {

public:
  RelationMember() : m_role("") {};

  RelationMember(std::string _m_type, osm_nwr_signed_id_t _m_ref, std::string _m_role) :
    m_role(_m_role), m_ref(_m_ref), m_type(_m_type) {};

  void set_type(const char *type) {

    std::string t = std::string(type);

    if (boost::iequals(t, "Node"))
      m_type = "Node";
    else if (boost::iequals(t, "Way"))
      m_type = "Way";
    else if (boost::iequals(t, "Relation"))
      m_type = "Relation";
    else
      throw xml_error(
          fmt::format("Invalid type {} in member relation", type));
  }

  void set_role(const char *role) {

    if (unicode_strlen(role) > 255) {
      throw xml_error(
          "Relation Role has more than 255 unicode characters");
    }

    m_role = std::string(role);
  }

  void set_ref(const char *ref) {

    osm_nwr_signed_id_t _ref = 0;

    try {
      _ref = std::stol(ref);
    } catch (std::invalid_argument &e) {
      throw xml_error("Relation member 'ref' attribute is not numeric");
    } catch (std::out_of_range &e) {
      throw xml_error(
          "Relation member 'ref' attribute value is too large");
    }

    if (_ref == 0) {
      throw xml_error("Relation member 'ref' attribute may not be 0");
    }

    m_ref = _ref;
  }

  bool is_valid() {

    if (!m_type)
      throw xml_error("Missing 'type' attribute in Relation member");

    if (!m_ref)
      throw xml_error("Missing 'ref' attribute in Relation member");

    return (m_ref && m_type);
  }

  std::string type() const { return *m_type; }

  std::string role() const { return m_role; }

  osm_nwr_signed_id_t ref() const { return *m_ref; }

private:
  std::string m_role;
  std::optional<osm_nwr_signed_id_t> m_ref;
  std::optional<std::string> m_type;
};

class Relation : public OSMObject {
public:
  Relation() : OSMObject() {};

  virtual ~Relation() = default;

  void add_member(RelationMember &member) {
    if (!member.is_valid())
      throw xml_error(
          "Relation member does not include all mandatory fields");
    m_relation_member.emplace_back(member);
  }

  const std::vector<RelationMember> &members() const {
    return m_relation_member;
  }

  std::string get_type_name() { return "Relation"; }

  bool is_valid(operation op) const {

    switch (op) {

    case operation::op_delete:
      return (is_valid());

    default:
      if ((global_settings::get_relation_max_members()) &&
	  m_relation_member.size() > *global_settings::get_relation_max_members()) {
        throw http::bad_request(
             fmt::format("You tried to add {:d} members to relation {:d}, however only {:d} are allowed",
        	m_relation_member.size(),
        	(has_id() ? id() : 0),
        	*global_settings::get_relation_max_members()));
      }

      return (is_valid());
    }
  }

private:
  std::vector<RelationMember> m_relation_member;
  using OSMObject::is_valid;
};

} // namespace api06

#endif
