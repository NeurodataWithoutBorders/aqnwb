#pragma once

#include <memory>
#include <string>
#include <utility>

#include "Types.hpp"
#include "io/BaseIO.hpp"

namespace AQNWB::NWB
{

/**
 * @brief Base class for the specification of a neurodata_type instance.
 *
 * A spec bundles all the values needed to initialize an instance of a
 * RegisteredType (e.g., attributes, dataset configurations, and links to other
 * objects). Each RegisteredType subclass `T` defines a nested `T::Spec` struct
 * that inherits from the spec of its parent type (mirroring the inheritance
 * defined in the schema) and is passed to `T::initialize(const T::Spec&)`.
 *
 * Separating the specification from the initialization makes it possible to
 * create, inspect, modify, and validate the configuration of an object before
 * any data is written to the file.
 *
 * Specs are defined using the macros `REGISTER_SPEC`, `DEFINE_SPEC_FIELD`, and
 * `INHERIT_SPEC_SETTER`, e.g.:
 *
 * @code
 * class MyType : public NWBContainer
 * {
 * public:
 *   REGISTER_SUBCLASS(MyType, NWBContainer, "my-namespace")
 *
 *   struct Spec : public NWBContainer::Spec
 *   {
 *     REGISTER_SPEC(MyType, NWBContainer::Spec)
 *     /// @brief Description of the object.
 *     DEFINE_SPEC_FIELD(Spec, std::string, description, withDescription, "")
 *   };
 *
 *   Status initialize(const Spec& spec);
 * };
 * @endcode
 */
struct RegisteredTypeSpec
{
  /**
   * @brief Virtual destructor.
   */
  virtual ~RegisteredTypeSpec() = default;

  /**
   * @brief Get the name of the neurodata_type that this spec describes.
   * @return The name of the type, e.g., "TimeSeries".
   */
  virtual std::string getTypeName() const = 0;

  /**
   * @brief Get the schema namespace of the neurodata_type that this spec
   * describes.
   * @return The namespace of the type, e.g., "core".
   */
  virtual std::string getNamespace() const = 0;

  /**
   * @brief Get the full name of the type, including the namespace.
   * @return The full type name, e.g., "core::TimeSeries".
   */
  inline std::string getFullTypeName() const
  {
    return getNamespace() + "::" + getTypeName();
  }

  /**
   * @brief Validate the values of the spec against the schema.
   *
   * Validation checks that are possible without access to the file, e.g.,
   * that required values are set or that a link target is not null.
   * Overrides must call the `validate` method of their base spec first.
   *
   * @return Status::Success if the spec is valid, otherwise Status::Failure.
   */
  virtual Types::Status validate() const { return Types::Status::Success; }
};

}  // namespace AQNWB::NWB

/**
 * @brief Macro to define the type information of a spec nested in a
 * RegisteredType.
 *
 * This macro must be used inside the definition of the nested `OWNER::Spec`
 * struct. It defines:
 * - `OwnerType` and `BaseSpecType` type aliases
 * - overrides of `getTypeName` and `getNamespace` that return the values
 *   registered for `OWNER` via `REGISTER_SUBCLASS`
 * - `createAndInitialize(path, io)` to create an `OWNER` object at the given
 *   path and initialize it with this spec. Returns nullptr if the creation or
 *   initialization fails.
 *
 * @note
 * The Doxyfile.in defines a simplified expansion of this macro for generating
 * the documentation, so changes to the macro should be reflected there as
 * well.
 *
 * @param OWNER The RegisteredType subclass that the spec describes.
 * @param BASE_SPEC The spec of the parent type of OWNER.
 */
#define REGISTER_SPEC(OWNER, BASE_SPEC) \
  using OwnerType = OWNER; \
  using BaseSpecType = BASE_SPEC; \
  std::string getTypeName() const override \
  { \
    return OWNER::getStaticTypeName(); \
  } \
  std::string getNamespace() const override \
  { \
    return OWNER::getStaticNamespace(); \
  } \
  std::shared_ptr<OWNER> createAndInitialize( \
      const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io) const \
  { \
    auto obj = OWNER::create(path, io); \
    if (obj == nullptr \
        || obj->initialize(*this) != AQNWB::Types::Status::Success) \
    { \
      return nullptr; \
    } \
    return obj; \
  }

/**
 * @brief Macro to define a field of a spec along with a setter method.
 *
 * The setter returns a reference to the spec to allow chaining of setters,
 * e.g., `MyType::Spec().withDescription("...").withLocation("...")`.
 *
 * Required fields should use a plain type. Optional fields should use
 * `std::optional<T>` with `std::nullopt` as the default value, so that the
 * `initialize` method only writes the field if it has been set.
 *
 * The field is documented via a regular Doxygen comment block preceding the
 * macro, e.g.:
 *
 * @code
 * /// @brief Location of the electrode group. Required.
 * DEFINE_SPEC_FIELD(Spec, std::string, location, withLocation, "")
 * @endcode
 *
 * @note
 * The Doxyfile.in defines a simplified expansion of this macro for generating
 * the documentation, so changes to the macro should be reflected there as
 * well.
 *
 * @param SPEC The spec type that the field is defined in.
 * @param TYPE The type of the field. Types that contain commas must be
 * defined via a type alias first.
 * @param MEMBER The name of the member variable.
 * @param SETTER The name of the setter method, e.g., `withDescription`.
 * @param ... The default value of the field, e.g., `""`, `std::nullopt`,
 * or `{1, 2}`.
 */
#define DEFINE_SPEC_FIELD(SPEC, TYPE, MEMBER, SETTER, ...) \
  TYPE MEMBER = __VA_ARGS__; \
  SPEC& SETTER(TYPE value) \
  { \
    MEMBER = std::move(value); \
    return *this; \
  }

/**
 * @brief Macro to re-expose a setter of a base spec with the derived spec as
 * return type.
 *
 * Setters defined via `DEFINE_SPEC_FIELD` return a reference to the spec they
 * are defined in. To be able to chain setters of a derived spec with the
 * setters inherited from a base spec, the derived spec re-exposes the
 * inherited setters via this macro.
 *
 * @note
 * The Doxyfile.in defines a simplified expansion of this macro for generating
 * the documentation, so changes to the macro should be reflected there as
 * well.
 *
 * @param SPEC The derived spec type.
 * @param BASE_SPEC The base spec type that defines the setter.
 * @param SETTER The name of the setter method.
 */
#define INHERIT_SPEC_SETTER(SPEC, BASE_SPEC, SETTER) \
  template<typename... Args> \
  SPEC& SETTER(Args&&... args) \
  { \
    BASE_SPEC::SETTER(std::forward<Args>(args)...); \
    return *this; \
  }
