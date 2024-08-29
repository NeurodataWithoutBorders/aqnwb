#pragma once

#include <string>
#include <string_view>
#include <array>

namespace AQNWB::spec::hdmf_common
{

const std::string version = "1.8.0";

constexpr std::string_view base = R"delimiter(
{"datasets":[{"data_type_def":"Data","doc":"An abstract data type for a dataset."}],"groups":[{"data_type_def":"Container","doc":"An abstract data type for a group storing collections of data and metadata. Base type for all data and metadata containers."},{"data_type_def":"SimpleMultiContainer","data_type_inc":"Container","doc":"A simple Container for holding onto multiple containers.","datasets":[{"data_type_inc":"Data","quantity":"*","doc":"Data objects held within this SimpleMultiContainer."}],"groups":[{"data_type_inc":"Container","quantity":"*","doc":"Container objects held within this SimpleMultiContainer."}]}]})delimiter";

constexpr std::string_view table = R"delimiter(
{"datasets":[{"data_type_def":"VectorData","data_type_inc":"Data","doc":"An n-dimensional dataset representing a column of a DynamicTable. If used without an accompanying VectorIndex, first dimension is along the rows of the DynamicTable and each step along the first dimension is a cell of the larger table. VectorData can also be used to represent a ragged array if paired with a VectorIndex. This allows for storing arrays of varying length in a single cell of the DynamicTable by indexing into this VectorData. The first vector is at VectorData[0:VectorIndex[0]]. The second vector is at VectorData[VectorIndex[0]:VectorIndex[1]], and so on.","dims":[["dim0"],["dim0","dim1"],["dim0","dim1","dim2"],["dim0","dim1","dim2","dim3"]],"shape":[[null],[null,null],[null,null,null],[null,null,null,null]],"attributes":[{"name":"description","dtype":"text","doc":"Description of what these vectors represent."}]},{"data_type_def":"VectorIndex","data_type_inc":"VectorData","dtype":"uint8","doc":"Used with VectorData to encode a ragged array. An array of indices into the first dimension of the target VectorData, and forming a map between the rows of a DynamicTable and the indices of the VectorData. The name of the VectorIndex is expected to be the name of the target VectorData object followed by \"_index\".","dims":["num_rows"],"shape":[null],"attributes":[{"name":"target","dtype":{"target_type":"VectorData","reftype":"object"},"doc":"Reference to the target dataset that this index applies to."}]},{"data_type_def":"ElementIdentifiers","data_type_inc":"Data","default_name":"element_id","dtype":"int","dims":["num_elements"],"shape":[null],"doc":"A list of unique identifiers for values within a dataset, e.g. rows of a DynamicTable."},{"data_type_def":"DynamicTableRegion","data_type_inc":"VectorData","dtype":"int","doc":"DynamicTableRegion provides a link from one table to an index or region of another. The `table` attribute is a link to another `DynamicTable`, indicating which table is referenced, and the data is int(s) indicating the row(s) (0-indexed) of the target array. `DynamicTableRegion`s can be used to associate rows with repeated meta-data without data duplication. They can also be used to create hierarchical relationships between multiple `DynamicTable`s. `DynamicTableRegion` objects may be paired with a `VectorIndex` object to create ragged references, so a single cell of a `DynamicTable` can reference many rows of another `DynamicTable`.","dims":["num_rows"],"shape":[null],"attributes":[{"name":"table","dtype":{"target_type":"DynamicTable","reftype":"object"},"doc":"Reference to the DynamicTable object that this region applies to."},{"name":"description","dtype":"text","doc":"Description of what this table region points to."}]}],"groups":[{"data_type_def":"DynamicTable","data_type_inc":"Container","doc":"A group containing multiple datasets that are aligned on the first dimension (Currently, this requirement if left up to APIs to check and enforce). These datasets represent different columns in the table. Apart from a column that contains unique identifiers for each row, there are no other required datasets. Users are free to add any number of custom VectorData objects (columns) here. DynamicTable also supports ragged array columns, where each element can be of a different size. To add a ragged array column, use a VectorIndex type to index the corresponding VectorData type. See documentation for VectorData and VectorIndex for more details. Unlike a compound data type, which is analogous to storing an array-of-structs, a DynamicTable can be thought of as a struct-of-arrays. This provides an alternative structure to choose from when optimizing storage for anticipated access patterns. Additionally, this type provides a way of creating a table without having to define a compound type up front. Although this convenience may be attractive, users should think carefully about how data will be accessed. DynamicTable is more appropriate for column-centric access, whereas a dataset with a compound type would be more appropriate for row-centric access. Finally, data size should also be taken into account. For small tables, performance loss may be an acceptable trade-off for the flexibility of a DynamicTable.","attributes":[{"name":"colnames","dtype":"text","dims":["num_columns"],"shape":[null],"doc":"The names of the columns in this table. This should be used to specify an order to the columns."},{"name":"description","dtype":"text","doc":"Description of what is in this dynamic table."}],"datasets":[{"name":"id","data_type_inc":"ElementIdentifiers","dtype":"int","dims":["num_rows"],"shape":[null],"doc":"Array of unique identifiers for the rows of this dynamic table."},{"data_type_inc":"VectorData","doc":"Vector columns, including index columns, of this dynamic table.","quantity":"*"}]},{"data_type_def":"AlignedDynamicTable","data_type_inc":"DynamicTable","doc":"DynamicTable container that supports storing a collection of sub-tables. Each sub-table is a DynamicTable itself that is aligned with the main table by row index. I.e., all DynamicTables stored in this group MUST have the same number of rows. This type effectively defines a 2-level table in which the main data is stored in the main table implemented by this type and additional columns of the table are grouped into categories, with each category being represented by a separate DynamicTable stored within the group.","attributes":[{"name":"categories","dtype":"text","dims":["num_categories"],"shape":[null],"doc":"The names of the categories in this AlignedDynamicTable. Each category is represented by one DynamicTable stored in the parent group. This attribute should be used to specify an order of categories and the category names must match the names of the corresponding DynamicTable in the group."}],"groups":[{"data_type_inc":"DynamicTable","doc":"A DynamicTable representing a particular category for columns in the AlignedDynamicTable parent container. The table MUST be aligned with (i.e., have the same number of rows) as all other DynamicTables stored in the AlignedDynamicTable parent container. The name of the category is given by the name of the DynamicTable and its description by the description attribute of the DynamicTable.","quantity":"*"}]}]})delimiter";

constexpr std::string_view sparse = R"delimiter(
{"groups":[{"data_type_def":"CSRMatrix","data_type_inc":"Container","doc":"A compressed sparse row matrix. Data are stored in the standard CSR format, where column indices for row i are stored in indices[indptr[i]:indptr[i+1]] and their corresponding values are stored in data[indptr[i]:indptr[i+1]].","attributes":[{"name":"shape","dtype":"uint","dims":["number of rows, number of columns"],"shape":[2],"doc":"The shape (number of rows, number of columns) of this sparse matrix."}],"datasets":[{"name":"indices","dtype":"uint","dims":["number of non-zero values"],"shape":[null],"doc":"The column indices."},{"name":"indptr","dtype":"uint","dims":["number of rows in the matrix + 1"],"shape":[null],"doc":"The row index pointer."},{"name":"data","dims":["number of non-zero values"],"shape":[null],"doc":"The non-zero values in the matrix."}]}]})delimiter";

constexpr std::string_view namespaces = R"delimiter(
{"namespaces":[{"name":"hdmf-common","doc":"Common data structures provided by HDMF","author":["Andrew Tritt","Oliver Ruebel","Ryan Ly","Ben Dichter"],"contact":["ajtritt@lbl.gov","oruebel@lbl.gov","rly@lbl.gov","bdichter@lbl.gov"],"full_name":"HDMF Common","schema":[{"source":"base"},{"source":"table"},{"source":"sparse"}],"version":"1.8.0"}]})delimiter";

constexpr std::array<std::pair<std::string_view, std::string_view>, 4> specVariables {{
  {"base", base},
  {"table", table},
  {"sparse", sparse},
  {"namespace", namespaces}
}};
}  // namespace AQNWB::spec::hdmf_common
