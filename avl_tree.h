#ifndef AVL_TREE_H  // Check if AVL_TREE_H is not defined, to prevent multiple inclusions of this header file
#define AVL_TREE_H  // Define AVL_TREE_H to prevent multiple inclusions in the future

#include <memory>  // Include memory management library for smart pointers (e.g., shared_ptr)
#include <string>  // Include the string library for string manipulation
#include <vector>  // Include the vector library for using dynamic arrays (not used directly, but often included for such purposes)
#include <fstream>  // Include the fstream library for file input/output operations
#include <unordered_map>  // Include the unordered_map library to use hash maps for efficient key-value storage

// Template function to save an unordered_map to a binary file
template<typename K, typename V>
void saveMap(const std::unordered_map<K, V>& map, std::ofstream& out) {
    size_t size = map.size();  // Get the size of the map
    out.write(reinterpret_cast<const char*>(&size), sizeof(size));  // Write the size of the map to the file

    // Iterate over each key-value pair in the map
    for (const auto& pair : map) {
        size_t keyLength = pair.first.length();  // Get the length of the key (string)
        out.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));  // Write key length to file
        out.write(pair.first.c_str(), keyLength);  // Write the key to the file
        out.write(reinterpret_cast<const char*>(&pair.second), sizeof(pair.second));  // Write the value to the file
    }
}

// Template function to load an unordered_map from a binary file
template<typename K, typename V>
void loadMap(std::unordered_map<K, V>& map, std::ifstream& in) {
    size_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size));  // Read the size of the map from the file
    map.clear();  // Clear the current map to reload data
    for (size_t i = 0; i < size; i++) {  // Loop to read each key-value pair from the file
        size_t keyLength;
        in.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));  // Read the length of the key
        std::string key(keyLength, '\0');  // Create a string to hold the key
        in.read(&key[0], keyLength);  // Read the key from the file into the string
        V value;  // Create a variable to hold the value
        in.read(reinterpret_cast<char*>(&value), sizeof(value));  // Read the value from the file
        map[key] = value;  // Insert the key-value pair into the map
    }
}

// Template class to represent a node in the AVL tree
template<typename T>
class AVLNode {
public:
    std::string key;  // Key of the node (unique identifier)
    T value;  // Value associated with the key
    int height;  // Height of the node, used for balancing the tree
    std::shared_ptr<AVLNode<T>> left;  // Pointer to the left child node
    std::shared_ptr<AVLNode<T>> right;  // Pointer to the right child node

    // Constructor for initializing a new node with key and value
    AVLNode(const std::string& k, const T& v)
        : key(k), value(v), height(1), left(nullptr), right(nullptr) {}

    // Method to save the node's data to a file
    void save(std::ofstream& out) const {
        if constexpr (std::is_same_v<T, std::unordered_map<std::string, int>>) {  // Check if value is a map
            saveMap(value, out);  // Save the map using the saveMap function
        }
    }

    // Method to load the node's data from a file
    void load(std::ifstream& in) {
        if constexpr (std::is_same_v<T, std::unordered_map<std::string, int>>) {  // Check if value is a map
            loadMap(value, in);  // Load the map using the loadMap function
        }
    }
};

// Template class to represent the AVL tree
template<typename T>
class AVLTree {
private:
    std::shared_ptr<AVLNode<T>> root;  // Root of the AVL tree

    // Helper method to get the height of a node
    int height(std::shared_ptr<AVLNode<T>> node) {
        if (!node) return 0;  // If the node is nullptr, return height as 0
        return node->height;  // Otherwise, return the height of the node
    }

    // Helper method to calculate the balance factor of a node (difference between left and right subtree heights)
    int getBalance(std::shared_ptr<AVLNode<T>> node) {
        if (!node) return 0;  // If the node is nullptr, balance is 0
        return height(node->left) - height(node->right);  // Return the difference in heights
    }

    // Helper method to perform a right rotation to rebalance the tree
    std::shared_ptr<AVLNode<T>> rightRotate(std::shared_ptr<AVLNode<T>> y) {
        auto x = y->left;  // Set x to be the left child of y
        auto T2 = x->right;  // Set T2 to be the right child of x

        x->right = y;  // Perform the right rotation by moving y to the right of x
        y->left = T2;  // Set T2 as the left child of y

        // Update the heights of the nodes after rotation
        y->height = std::max(height(y->left), height(y->right)) + 1;
        x->height = std::max(height(x->left), height(x->right)) + 1;

        return x;  // Return the new root of the subtree
    }

    // Helper method to perform a left rotation to rebalance the tree
    std::shared_ptr<AVLNode<T>> leftRotate(std::shared_ptr<AVLNode<T>> x) {
        auto y = x->right;  // Set y to be the right child of x
        auto T2 = y->left;  // Set T2 to be the left child of y

        y->left = x;  // Perform the left rotation by moving x to the left of y
        x->right = T2;  // Set T2 as the right child of x

        // Update the heights of the nodes after rotation
        x->height = std::max(height(x->left), height(x->right)) + 1;
        y->height = std::max(height(y->left), height(y->right)) + 1;

        return y;  // Return the new root of the subtree
    }

    // Helper method to insert a new node into the AVL tree
    std::shared_ptr<AVLNode<T>> insert(std::shared_ptr<AVLNode<T>> node, const std::string& key, const T& value) {
        if (!node) return std::make_shared<AVLNode<T>>(key, value);  // If the node is nullptr, create a new node

        if (key < node->key)  // If the key is smaller than the current node's key, insert into the left subtree
            node->left = insert(node->left, key, value);
        else if (key > node->key)  // If the key is greater than the current node's key, insert into the right subtree
            node->right = insert(node->right, key, value);
        else {
            node->value = value;  // If the key is equal, update the value and return the node
            return node;
        }

        node->height = 1 + std::max(height(node->left), height(node->right));  // Update the height of the node

        int balance = getBalance(node);  // Calculate the balance factor of the node

        // Perform rotations to rebalance the tree based on balance factor
        if (balance > 1 && key < node->left->key)
            return rightRotate(node);  // Left-heavy case, perform right rotation

        if (balance < -1 && key > node->right->key)
            return leftRotate(node);  // Right-heavy case, perform left rotation

        if (balance > 1 && key > node->left->key) {
            node->left = leftRotate(node->left);  // Left-right case, perform left rotation on left child
            return rightRotate(node);  // Then right rotation on the node
        }

        if (balance < -1 && key < node->right->key) {
            node->right = rightRotate(node->right);  // Right-left case, perform right rotation on right child
            return leftRotate(node);  // Then left rotation on the node
        }

        return node;  // Return the (possibly rebalanced) node
    }

    // Helper method to find a node by key
    std::shared_ptr<AVLNode<T>> find(std::shared_ptr<AVLNode<T>> node, const std::string& key) const {
        if (!node || node->key == key) return node;  // If the node is nullptr or the key matches, return the node

        if (key < node->key)  // If the key is smaller, search the left subtree
            return find(node->left, key);
        return find(node->right, key);  // Otherwise, search the right subtree
    }

    // Helper method to save the AVL tree to a file
    void save(std::ofstream& out) const {
        if (root) {
            root->save(out);  // If the tree is not empty, save the root node
        }
    }

    // Helper method to load the AVL tree from a file
    void load(std::ifstream& in) {
        if (root) {
            root->load(in);  // If the tree is not empty, load the root node
        }
    }

public:
    // Public method to insert a new node into the tree
    void insert(const std::string& key, const T& value) {
        root = insert(root, key, value);  // Insert into the tree and update the root
    }

    // Public method to find a node by key
    std::shared_ptr<AVLNode<T>> find(const std::string& key) const {
        return find(root, key);  // Find the node starting from the root
    }

    // Public method to save the tree to a file
    void saveToFile(const std::string& filename) const {
        std::ofstream out(filename, std::ios::binary);  // Open the file for binary writing
        save(out);  // Save the tree to the file
    }

    // Public method to load the tree from a file
    void loadFromFile(const std::string& filename) {
        std::ifstream in(filename, std::ios::binary);  // Open the file for binary reading
        load(in);  // Load the tree from the file
    }
};

#endif  // End the conditional inclusion to prevent multiple inclusions
