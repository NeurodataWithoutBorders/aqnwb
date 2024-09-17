#pragma once

#include <array>
#include <string>
#include <string_view>

namespace AQNWB::SPEC::HDMF_EXPERIMENTAL
{

const std::string version = "0.5.0";

constexpr std::string_view experimental = R"delimiter(
{"groups":[],"datasets":[{"data_type_def":"EnumData","data_type_inc":"VectorData","dtype":"uint8","doc":"Data that come from a fixed set of values. A data value of i corresponds to the i-th value in the VectorData referenced by the 'elements' attribute.","attributes":[{"name":"elements","dtype":{"target_type":"VectorData","reftype":"object"},"doc":"Reference to the VectorData object that contains the enumerable elements"}]}]})delimiter";

constexpr std::string_view resources = R"delimiter(
{"groups":[{"data_type_def":"HERD","data_type_inc":"Container","doc":"HDMF External Resources Data Structure. A set of six tables for tracking external resource references in a file or across multiple files.","datasets":[{"data_type_inc":"Data","name":"keys","doc":"A table for storing user terms that are used to refer to external resources.","dtype":[{"name":"key","dtype":"text","doc":"The user term that maps to one or more resources in the `resources` table, e.g., \"human\"."}],"dims":["num_rows"],"shape":[null]},{"data_type_inc":"Data","name":"files","doc":"A table for storing object ids of files used in external resources.","dtype":[{"name":"file_object_id","dtype":"text","doc":"The object id (UUID) of a file that contains objects that refers to external resources."}],"dims":["num_rows"],"shape":[null]},{"data_type_inc":"Data","name":"entities","doc":"A table for mapping user terms (i.e., keys) to resource entities.","dtype":[{"name":"entity_id","dtype":"text","doc":"The compact uniform resource identifier (CURIE) of the entity, in the form [prefix]:[unique local identifier], e.g., 'NCBI_TAXON:9606'."},{"name":"entity_uri","dtype":"text","doc":"The URI for the entity this reference applies to. This can be an empty string. e.g., https://www.ncbi.nlm.nih.gov/Taxonomy/Browser/wwwtax.cgi?mode=info&id=9606"}],"dims":["num_rows"],"shape":[null]},{"data_type_inc":"Data","name":"objects","doc":"A table for identifying which objects in a file contain references to external resources.","dtype":[{"name":"files_idx","dtype":"uint","doc":"The row index to the file in the `files` table containing the object."},{"name":"object_id","dtype":"text","doc":"The object id (UUID) of the object."},{"name":"object_type","dtype":"text","doc":"The data type of the object."},{"name":"relative_path","dtype":"text","doc":"The relative path from the data object with the `object_id` to the dataset or attribute with the value(s) that is associated with an external resource. This can be an empty string if the object is a dataset that contains the value(s) that is associated with an external resource."},{"name":"field","dtype":"text","doc":"The field within the compound data type using an external resource. This is used only if the dataset or attribute is a compound data type; otherwise this should be an empty string."}],"dims":["num_rows"],"shape":[null]},{"data_type_inc":"Data","name":"object_keys","doc":"A table for identifying which objects use which keys.","dtype":[{"name":"objects_idx","dtype":"uint","doc":"The row index to the object in the `objects` table that holds the key"},{"name":"keys_idx","dtype":"uint","doc":"The row index to the key in the `keys` table."}],"dims":["num_rows"],"shape":[null]},{"data_type_inc":"Data","name":"entity_keys","doc":"A table for identifying which keys use which entity.","dtype":[{"name":"entities_idx","dtype":"uint","doc":"The row index to the entity in the `entities` table."},{"name":"keys_idx","dtype":"uint","doc":"The row index to the key in the `keys` table."}],"dims":["num_rows"],"shape":[null]}]}]})delimiter";

constexpr std::string_view namespaces = R"delimiter(
{"namespaces":[{"name":"hdmf-experimental","doc":"Experimental data structures provided by HDMF. These are not guaranteed to be available in the future.","author":["Andrew Tritt","Oliver Ruebel","Ryan Ly","Ben Dichter","Matthew Avaylon"],"contact":["ajtritt@lbl.gov","oruebel@lbl.gov","rly@lbl.gov","bdichter@lbl.gov","mavaylon@lbl.gov"],"full_name":"HDMF Experimental","schema":[{"namespace":"hdmf-common"},{"source":"experimental"},{"source":"resources"}],"version":"0.5.0"}]})delimiter";

constexpr std::array<std::pair<std::string_view, std::string_view>, 3>
    specVariables {{{"experimental", experimental},
                    {"resources", resources},
                    {"namespace", namespaces}}};
}  // namespace AQNWB::SPEC::HDMF_EXPERIMENTAL
