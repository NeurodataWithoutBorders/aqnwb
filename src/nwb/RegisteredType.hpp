#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "io/BaseIO.hpp"

namespace AQNWB {
namespace NWB {

/**
 * @brief Base class for types defined in the NWB schema
 *
 * This class maintains a static registry of all subclasses that inherit from it.
 * To register a new class we need to call the macro "REGISTER_SUBCLASS(MySubClass)"
 * in the definition of the subclass. All subclasses must also implement the
 * constructor with the ``path`` and ``io`` as sole input arguments. This in turn
 * allows us to create any subclass of container based on its name because we
 * can : 1) look-up classes by their name in the registry and 2) all subclasses
 * have a consistent constructor. The registry is static in the sense that it
 * is a static member of the Container class (and as such it is shared among all
 * instances of Container and its subclasses). However, the registry is not static
 * in the sense that it is being created at compile time; but it is created and
 * can be modified at runtime.
 *
 */
class RegisteredType
{
public:
    /**
     * @brief Constructor.
     *
     * All registered subclasses of RegisteredType must implement a constructor
     * with these arguments.
     *
     * @param path The path of the registered type.
     * @param io A shared pointer to the IO object.
     */
    RegisteredType(const std::string& path, std::shared_ptr<IO::BaseIO> io);

    /**
     * @brief Destructor.
     */
    virtual ~RegisteredType();

    /**
     * @brief Gets the path of the registered type.
     * @return The path of the registered type.
     */
    std::string getPath() const;

    /**
     * @brief Get the registry of subclass names.
     *
     * @return A reference to an unordered_set containing the names of all registered subclasses.
     */
    static std::unordered_set<std::string>& getRegistry();

    /**
     * @brief Get the factory map for creating instances of subclasses.
     *
     * @return A reference to an unordered_map containing factory functions for registered subclasses.
     */
    static std::unordered_map<std::string, std::function<std::unique_ptr<RegisteredType>(const std::string&, std::shared_ptr<IO::BaseIO>)>>& getFactoryMap();

    /**
     * @brief Create an instance of a registered subclass by name.
     *
     * @param subclassName The name of the subclass to instantiate.
     * @param path The path of the registered type.
     * @param io A shared pointer to the IO object.
     * @return A unique_ptr to the created instance of the subclass, or nullptr if the subclass is not found.
     */
    static std::unique_ptr<RegisteredType> create(const std::string& subclassName, const std::string& path, std::shared_ptr<IO::BaseIO> io);

    /**
     * @brief Create an instance of a subclass of Container by type.
     *
     * @tparam T The subclass of Container to instantiate.
     * @param path The path of the container.
     * @param io A shared pointer to the IO object.
     * @return A unique_ptr to the created instance of the subclass.
     */
    template<typename T>
    static std::unique_ptr<T> create(const std::string& path, std::shared_ptr<IO::BaseIO> io) {
        static_assert(std::is_base_of<RegisteredType, T>::value, "T must be a derived class of RegisteredType");
        return std::unique_ptr<T>(new T(path, io));
    }

protected:
    /**
     * @brief Register a subclass name and its factory function in the registry.
     *
     * @param subclassName The name of the subclass to register.
     * @param factoryFunction The factory function to create instances of the subclass.
     */
    static void registerSubclass(const std::string& subclassName, std::function<std::unique_ptr<RegisteredType>(const std::string&, std::shared_ptr<IO::BaseIO>)> factoryFunction);

    /**
     * @brief The path of the registered type.
     */
    std::string path;

    /**
     * @brief A shared pointer to the IO object.
     */
    std::shared_ptr<IO::BaseIO> io;
};

/**
 * @brief Macro to register a subclass with the RegisteredType class registry.
 *
 * This macro defines a static method that triggers registration
 * of the subclass type when the subclass type is loaded.
 *
 * @param T The subclass type to register.
 */
#define REGISTER_SUBCLASS(T) \
    static bool registerSubclass() { \
        AQNWB::NWB::RegisteredType::registerSubclass(#T, [](const std::string& path, std::shared_ptr<IO::BaseIO> io) -> std::unique_ptr<AQNWB::NWB::RegisteredType> { return std::make_unique<T>(path, io); }); \
        return true; \
    } \
    static bool registered_;

}  // namespace NWB
}  // namespace AQNWB
