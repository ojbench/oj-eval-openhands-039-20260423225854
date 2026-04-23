#ifndef CSR_MATRIX_HPP
#define CSR_MATRIX_HPP

#include <vector>
#include <exception>

namespace sjtu {

class size_mismatch : public std::exception {
public:
    const char *what() const noexcept override {
        return "Size mismatch";
    }
};

class invalid_index : public std::exception {
public:
    const char *what() const noexcept override {
        return "Index out of range";
    }
};

// TODO: Implement a CSR matrix class
// You only need to implement the TODOs in this file
// DO NOT modify other parts of this file
// DO NOT include any additional headers
// DO NOT use STL other than std::vector

template <typename T>
class CSRMatrix {

private:
    // TODO: add your private member variables here
    size_t n_rows;
    size_t n_cols;
    std::vector<size_t> indptr;
    std::vector<size_t> indices;
    std::vector<T> data;
    
    // Helper method to find the position of element (i,j) in the data array
    size_t findPosition(size_t i, size_t j) const {
        if (i >= n_rows || j >= n_cols) {
            throw invalid_index();
        }
        
        size_t start = indptr[i];
        size_t end = indptr[i + 1];
        
        // Binary search for the column index j in the range [start, end)
        while (start < end) {
            size_t mid = start + (end - start) / 2;
            if (indices[mid] < j) {
                start = mid + 1;
            } else if (indices[mid] > j) {
                end = mid;
            } else {
                return mid; // Found the element
            }
        }
        
        return end; // Return the position where the element would be inserted
    }

public:
    // Assignment operators are deleted
    CSRMatrix &operator=(const CSRMatrix &other) = delete;
    CSRMatrix &operator=(CSRMatrix &&other) = delete;

    // Constructor for empty matrix with dimensions
    // TODO: Initialize an empty CSR matrix with n rows and m columns
    CSRMatrix(size_t n, size_t m) : n_rows(n), n_cols(m), indptr(n + 1, 0) {
        // All elements are zero, so indptr is all zeros
    }

    // Constructor with pre-built CSR components
    // TODO: Initialize CSR matrix from existing CSR format data, validate sizes
    CSRMatrix(size_t n, size_t m, size_t count,
        const std::vector<size_t> &indptr, 
        const std::vector<size_t> &indices,
        const std::vector<T> &data) : n_rows(n), n_cols(m), indptr(indptr), indices(indices), data(data) {
        // Validate dimensions
        if (n == 0 || m == 0) {
            throw invalid_index();
        }
        if (indptr.size() != n + 1) {
            throw size_mismatch();
        }
        if (indices.size() != count || data.size() != count) {
            throw size_mismatch();
        }
        // Validate indptr values
        if (indptr[0] != 0) {
            throw size_mismatch();
        }
        for (size_t i = 0; i < n; ++i) {
            if (indptr[i] > indptr[i + 1]) {
                throw size_mismatch();
            }
        }
        // Validate indices values
        for (size_t i = 0; i < count; ++i) {
            if (indices[i] >= m) {
                throw invalid_index();
            }
        }
    }

    // Copy constructor
    CSRMatrix(const CSRMatrix &other) = default;

    // Move constructor
    CSRMatrix(CSRMatrix &&other) = default;

    // Constructor from dense matrix format (given as vector of vectors)
    // TODO: Convert dense matrix representation to CSR format
    CSRMatrix(size_t n, size_t m, const std::vector<std::vector<T>> &dense_data) : n_rows(n), n_cols(m) {
        if (n == 0 || m == 0) {
            throw invalid_index();
        }
        if (dense_data.size() != n) {
            throw size_mismatch();
        }
        for (const auto &row : dense_data) {
            if (row.size() != m) {
                throw size_mismatch();
            }
        }
        
        // Count non-zero elements and build indptr
        indptr.resize(n + 1, 0);
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < m; ++j) {
                if (dense_data[i][j] != T{}) {  // Check if not default value
                    ++indptr[i + 1];
                }
            }
        }
        
        // Convert indptr to cumulative sum
        for (size_t i = 1; i <= n; ++i) {
            indptr[i] += indptr[i - 1];
        }
        
        // Resize indices and data arrays
        size_t nnz = indptr[n];
        indices.resize(nnz);
        data.resize(nnz);
        
        // Fill indices and data arrays
        size_t pos = 0;
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < m; ++j) {
                if (dense_data[i][j] != T{}) {
                    indices[pos] = j;
                    data[pos] = dense_data[i][j];
                    ++pos;
                }
            }
        }
    }

    // Destructor
    ~CSRMatrix() = default;

    // Get dimensions and non-zero count
    // TODO: Return the number of rows
    size_t getRowSize() const {
        return n_rows;
    }

    // TODO: Return the number of columns
    size_t getColSize() const {
        return n_cols;
    }

    // TODO: Return the count of non-zero elements
    size_t getNonZeroCount() const {
        return data.size();
    }

    // Element access
    // TODO: Retrieve element at position (i,j)
    T get(size_t i, size_t j) const {
        if (i >= n_rows || j >= n_cols) {
            throw invalid_index();
        }
        
        size_t pos = findPosition(i, j);
        size_t start = indptr[i];
        size_t end = indptr[i + 1];
        
        // If pos is within the range and the column index matches, return the value
        if (pos < end && indices[pos] == j) {
            return data[pos];
        }
        
        // Otherwise, return default-constructed value (zero for numeric types)
        return T{};
    }

    // TODO: Set element at position (i,j), updating CSR structure as needed
    void set(size_t i, size_t j, const T &value) {
        if (i >= n_rows || j >= n_cols) {
            throw invalid_index();
        }
        
        size_t pos = findPosition(i, j);
        size_t start = indptr[i];
        size_t end = indptr[i + 1];
        
        // If the element already exists
        if (pos < end && indices[pos] == j) {
            if (value == T{}) {
                // If setting to zero, we would normally remove the element
                // But according to the problem update: "set(i,j,value) 无需删除元素."
                // So we just set the value to zero
                data[pos] = value;
            } else {
                // Update the existing value
                data[pos] = value;
            }
        } else {
            // Element doesn't exist, need to insert
            // But according to the problem update: "set(i,j,value) 无需删除元素."
            // And we're not required to handle insertion of new non-zero elements
            // However, to maintain correctness, we should handle setting non-zero values
            if (value != T{}) {
                // Insert the new non-zero element
                indices.insert(indices.begin() + pos, j);
                data.insert(data.begin() + pos, value);
                
                // Update indptr for all rows after i
                for (size_t row = i + 1; row <= n_rows; ++row) {
                    indptr[row]++;
                }
            }
            // If setting to zero and element doesn't exist, do nothing
        }
    }

    // Access CSR components
    // TODO: Return the row pointer array
    const std::vector<size_t> &getIndptr() const {
        return indptr;
    }

    // TODO: Return the column indices array
    const std::vector<size_t> &getIndices() const {
        return indices;
    }

    // TODO: Return the data values array
    const std::vector<T> &getData() const {
        return data;
    }

    // Convert to dense matrix format
    // TODO: Convert CSR format to dense matrix representation
    std::vector<std::vector<T>> getMatrix() const {
        std::vector<std::vector<T>> result(n_rows, std::vector<T>(n_cols, T{}));
        
        for (size_t i = 0; i < n_rows; ++i) {
            size_t start = indptr[i];
            size_t end = indptr[i + 1];
            for (size_t j = start; j < end; ++j) {
                result[i][indices[j]] = data[j];
            }
        }
        
        return result;
    }

    // Matrix-vector multiplication
    // TODO: Implement multiplication of this matrix with vector vec
    std::vector<T> operator*(const std::vector<T> &vec) const {
        if (vec.size() != n_cols) {
            throw size_mismatch();
        }
        
        std::vector<T> result(n_rows, T{});
        
        for (size_t i = 0; i < n_rows; ++i) {
            size_t start = indptr[i];
            size_t end = indptr[i + 1];
            for (size_t j = start; j < end; ++j) {
                result[i] += data[j] * vec[indices[j]];
            }
        }
        
        return result;
    }

    // Row slicing
    // TODO: Extract submatrix containing rows [l,r)
    CSRMatrix getRowSlice(size_t l, size_t r) const {
        if (l >= r || r > n_rows) {
            throw invalid_index();
        }
        
        // Calculate the number of non-zero elements in the slice
        size_t count = indptr[r] - indptr[l];
        
        // Create new indptr array for the slice
        std::vector<size_t> new_indptr(r - l + 1);
        for (size_t i = 0; i <= r - l; ++i) {
            new_indptr[i] = indptr[l + i] - indptr[l];
        }
        
        // Create new indices and data arrays
        std::vector<size_t> new_indices(count);
        std::vector<T> new_data(count);
        
        for (size_t i = 0; i < count; ++i) {
            new_indices[i] = indices[indptr[l] + i];
            new_data[i] = data[indptr[l] + i];
        }
        
        return CSRMatrix(r - l, n_cols, count, new_indptr, new_indices, new_data);
    }
};

}

#endif // CSR_MATRIX_HPP