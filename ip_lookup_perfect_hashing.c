// Roll No. : CS22B2053
// Name     : Dhivya Dharshan V
// ADSA TUTORIAL - 1

// <-----| IP Lookup (Longest Prefix Match) |----->

// NOTE: This code is done in a Windows system.
// When compiling a C program with gcc on Windows, use the -l option to specify libraries to link against. 
// The -lws2_32 flag in the gcc command is used to link my program with the Winsock 2 library.

// To compile: gcc -o ip_lookup ip_lookup_perfect_hashing.c -lws2_32
// To execute: ./ip_lookup.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> // For winsock functions

#pragma comment(lib, "ws2_32.lib") // Link with winsock library

#define PRIME 31 // Prime number for hash function 
// (taking smaller prime numbers resulted in more collisions in the first-level)

typedef struct { // Structrue to store IP prefix and next hop information
                 // (consider next_hop to be more of an identifier for a prefix)
    unsigned int prefix;  // Binary representation of the IP prefix
    int prefix_len;       // Length of the prefix (eg: 24 for /24)
    int next_hop;         // Next hop associated with this prefix 
} Prefix_Entry;

// Hash table size
#define TABLE_SIZE 5  // Smaller size for fewer prefixes

Prefix_Entry* first_lvl_table[TABLE_SIZE]; // First-level hash table (array of buckets)

Prefix_Entry** second_lvl_table[TABLE_SIZE]; // Second-level hash tables
int second_lvl_size[TABLE_SIZE];  // sizes of second-level tables

int hash_a[TABLE_SIZE]; // Store 'a' and 'b' for second-level hash functions
int hash_b[TABLE_SIZE];

int hash_first_lvl(unsigned int prefix) { // Simple hash function for first-level table
    return (prefix % PRIME) % TABLE_SIZE;
}

// Function to calculate perfect hash for second-level tables
int hash_second_lvl(unsigned int prefix, int a, int b, int table_size) {
    return ((a * prefix + b) % PRIME) % table_size;
}

// WHY USE PRIME NUMBER?
// This is because prime numbers have no divisors other than 1 and themselves, 
// which minimizes the risk of systematic collisions.

// Avoiding Regular Patterns:
// If the table size is not a prime number, it can introduce regular patterns in the hash values, 
// especially if the hash function’s coefficients (like 'a' and 'b') and 
// the key distribution align in a specific way.
// Example: Consider a hash function h(key) = key % N where N is 'not' a prime number. 
// If N has divisors, then the distribution of key % N might be biased towards values 
// that are multiples of those divisors, leading to "clustering".

// Example of Uniform Distribution with Primes:
// Consider two hash functions:
// Non-Prime Table Size: Suppose table_size = 6, and we use h(key) = key % 6. 
// If we input keys like 1, 2, 7, 8, we might see patterns: 1 % 6 = 1, 2 % 6 = 2, 7 % 6 = 1, 8 % 6 = 2. 
// This shows clustering in indices 1 and 2.
// Prime Table Size: Suppose table_size = 7, and we use h(key) = key % 7. 
// For the same keys 1, 2, 7, 8, the results would be: 1 % 7 = 1, 2 % 7 = 2, 7 % 7 = 0, 8 % 7 = 1. 
// Although this still shows some clustering, with a larger prime size (my table size is 31), 
// the distribution becomes more uniform and less predictable.

// Convert dotted-decimal IP to binary format (unsigned int) using inet_addr
unsigned int ip_to_bin(const char *ip_str) {
    unsigned int ip_bin = inet_addr(ip_str);  // Convert dotted-decimal IP to binary
    if(ip_bin == INADDR_NONE) {
        printf("\n------Invalid IP address: %s------\n\n", ip_str);
        exit(1);
    }
    return ntohl(ip_bin);  // Convert network byte order to host byte order 
                           // (based on the local system's ordering preference)
}

// Byte Order (Endianness)
// Network Byte Order: Big-endian. The most significant byte (the "big end") 
// is stored at the smallest address. This is the standard byte order used in network protocols.
// Host Byte Order: This can be either big-endian or little-endian depending on the system architecture. 
// Little-endian systems store the least significant byte (the "little end") at the smallest address.

// Why Conversion is Necessary
// When working with network protocols, data is typically transmitted in network byte order. 
// However, the local machine might use a different byte order. For consistent processing and 
// comparison of data, you need to convert the data from network byte order to host byte order.

// Network byte order (big-endian): 11000000 10101000 00000001 01100100
// Host byte order (little-endian): 01100100 00000001 10101000 11000000

// Function to apply the prefix mask (eg: /24)
unsigned int apply_mask(unsigned int ip, int prefix_len) {
    return ip & (~0U << (32 - prefix_len));  // Apply prefix mask
}

// Example:
// IP    : 11000000.10101000.00000001.00000001  (192.168.1.1)
// Mask  : 11111111.11111111.11111111.00000000  (/24)
// --------------------------------------------------
// Result: 11000000.10101000.00000001.00000000  (192.168.1.0)

// Function to print IP stored in binary form, in dotted-decimal form
void print_ip(unsigned int ip) {
    printf("%u.%u.%u.%u\n",
        (ip >> 24) & 0xFF,
        (ip >> 16) & 0xFF,
        (ip >> 8) & 0xFF,
        ip & 0xFF);
}

// Example:
// Suppose the ip value is 3232235777 (which is 192.168.1.1 in binary):
// Binary Representation: 11000000 10101000 00000001 00000001
// Extracting Each Byte:
// First Octet : (ip >> 24) & 0xFF => 11000000 & 0xFF => 192
// Second Octet: (ip >> 16) & 0xFF => 10101000 & 0xFF => 168
// Third Octet : (ip >> 8) & 0xFF => 00000001 & 0xFF => 1
// Fourth Octet: ip & 0xFF => 00000001 & 0xFF => 1
// The output of print_ip(3232235777) will be 192.168.1.1.

// Function to insert a prefix into the first and second-level tables
void insert_prefix(const char *ip_str, int prefix_len, int next_hop) {
    unsigned int ip_bin = ip_to_bin(ip_str); // Convert to binary
    unsigned int masked_ip = apply_mask(ip_bin, prefix_len); // Mask the prefix

    Prefix_Entry* entry = (Prefix_Entry*)malloc(sizeof(Prefix_Entry)); // Dynamic memory allocation
    entry->prefix = masked_ip;
    entry->prefix_len = prefix_len;
    entry->next_hop = next_hop;

    int bucket = hash_first_lvl(masked_ip);
    
    // If no second-level table exists for this bucket, create it
    if(second_lvl_size[bucket] == 0) {
        // Generate random values for 'a' and 'b'
        int a = rand() % PRIME;
        int b = rand() % PRIME;
        hash_a[bucket] = a; // Store the values of 'a' and 'b' specific to this bucket
        hash_b[bucket] = b;
        
        // Set size to n^2 (index ranging from 0 to n^2 - 1) where 'n' is the no.of prefixes 
        // (no.of collisions at first-level, in that particular bucket), n = 1 for first entry
        second_lvl_size[bucket] = 1;
        second_lvl_table[bucket] = (Prefix_Entry**)malloc(second_lvl_size[bucket] * sizeof(Prefix_Entry*));
        int second_lvl_index = hash_second_lvl(entry->prefix, a, b, second_lvl_size[bucket]);
        second_lvl_table[bucket][second_lvl_index] = entry;  // Insert first prefix directly

        // Printing the location information of the prefix in the hash table
        printf("\nPrefix: ");
        print_ip(entry->prefix);
        printf("Bucket: %d | a: %d | b: %d | Index (in second-level table): 0 | Second level size: %d\n", bucket, a, b, second_lvl_size[bucket]);
        return;
    }

    int n = second_lvl_size[bucket]; // Current no.of entries
    Prefix_Entry* new_table[n + 1]; // Creating new table to store the old table's entries while we modify it
    // 'n + 1' size, so that we can add the lastest entry as well
    new_table[n + 1] = (Prefix_Entry*)malloc(n + 1 * sizeof(Prefix_Entry));
    for(int i = 0; i < n; i++) {
        if(second_lvl_table[bucket][i]) {
            new_table[i] = second_lvl_table[bucket][i];
        }
    }
    new_table[n] = entry;

    // Resize second-level table based on no.of collisions
    int new_n = (n + 1) * (n + 1); 
    second_lvl_size[bucket] = new_n; // Increase size to (n + 1) ^ 2
    second_lvl_table[bucket] = (Prefix_Entry**)realloc(second_lvl_table[bucket], second_lvl_size[bucket] * sizeof(Prefix_Entry*));

    // Find a perfect hash function that avoids collisions
    int collision = 1;
    while(collision) {
        for(int i = 0; i < second_lvl_size[bucket]; i++) {
            second_lvl_table[bucket][i] = NULL;  // Clear all entries
        }
        collision = 0;
        int a = hash_a[bucket];
        int b = hash_b[bucket];

        // Rehash all the old entries and hash the latest entry
        printf("\n------(COLLISION) REHASHING NOW------\n");
        for(int i = 0; i < n + 1; i++) { 
            if(new_table[i]) { 
                // Getting new index position
                int new_index = hash_second_lvl(new_table[i]->prefix, a, b, new_n);
                if(second_lvl_table[bucket][new_index]) { // If prefix already exists at that index (collision)
                    collision = 1;
                    a = rand() % PRIME;  // Try new values for 'a' and 'b'
                    b = rand() % PRIME;
                    hash_a[bucket] = a;
                    hash_b[bucket] = b;
                    break; 
                } else {
                    second_lvl_table[bucket][new_index] = new_table[i];
                    // Printing the location information of the prefix in the hash table
                    printf("\nPrefix: ");
                    print_ip(new_table[i]->prefix);
                    printf("Bucket: %d | a: %d | b: %d | Index (in second-level table): %d | Second level size: %d\n", bucket, a, b, new_index, second_lvl_size[bucket]);
                }
            }
        }
        hash_a[bucket] = a; // Store the final 'a' and 'b' values for future ip lookup
        hash_b[bucket] = b;
    }
}

// Function to perform the IP lookup with Longest Prefix Match
unsigned int ip_lookup(const char *ip_str) {
    unsigned int ip_bin = ip_to_bin(ip_str); // Convert to binary
    int longest_prefix_len = -1;  // To track the longest matching prefix
    unsigned int answer = -1;     // To store the longest matching prefix

    // Start checking from the highest prefix length possible (32 bits),
    // if match not found, then decrease to 24 and so on...
    for(int prefix_len = 32; prefix_len >= 8; prefix_len -= 8) {
        // Apply the prefix mask based on the current prefix length
        unsigned int masked_ip = apply_mask(ip_bin, prefix_len);
        
        // Use first-level hash function on the masked IP to find the bucket
        int first_lvl_bucket = hash_first_lvl(masked_ip);
        printf("\nFirst-level index: %d", first_lvl_bucket);
        // If no second-level table exists for this , skip to next
        if(second_lvl_size[first_lvl_bucket] == 0) {
            printf("\n------EMPTY SLOT------\n");
            continue;
        }

        // Use the stored 'a' and 'b' for the second-level hash function
        int a = hash_a[first_lvl_bucket];
        int b = hash_b[first_lvl_bucket];
        printf("\na: %d | b: %d", a, b);

        // Use second-level hash function to find the index in the second-level table
        int second_lvl_index = hash_second_lvl(masked_ip, a, b, second_lvl_size[first_lvl_bucket]);
        printf("\nSecond-level index: %d", second_lvl_index);

        // Retrieve the entry from the second-level table
        Prefix_Entry *entry = second_lvl_table[first_lvl_bucket][second_lvl_index];
        // Check if the entry is a valid match for the current prefix length
        if((entry != NULL) && (entry->prefix_len == prefix_len) && (entry->prefix == masked_ip)) {
            printf("\nMatch found: ");
            print_ip(entry->prefix);
            longest_prefix_len = prefix_len; 
            answer = entry->prefix; // Update the longest matching prefix
            return answer; // The first match will be the longest match, so return it
        }
        printf("\n------NO MATCH------\n");
    }

    return answer;  // Return the IP for the longest matching prefix
}

int main() {
    // Winsock is required for network-related functionality that specifically deals with IP addresses 
    // and protocols in a Windows environment.
    // Initialize Winsock
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // version 2.2 and pointer to WSADATA
        printf("Failed to initialize Winsock\n");
        return 1;
    }

    // User-Input
    int num_prefixes;
    printf("\nEnter the no.of prefixes (IPs) to insert: ");
    scanf("%d", &num_prefixes);

    for(int i = 0; i < num_prefixes; i++) {
        char ip_str[16]; // IP string length will not exceed 16
        int prefix_len, next_hop; // (consider next_hop to be more of an identifier for a prefix)

        printf("\nEnter IP prefix %d (eg: 192.168.1.0): ", i + 1);
        scanf("%s", ip_str);
        printf("Enter prefix length (eg: 24 for /24): ");
        scanf("%d", &prefix_len);
        printf("Enter next hop (positive integer): ");
        scanf("%d", &next_hop);

        insert_prefix(ip_str, prefix_len, next_hop);
    }

    char ip_to_lookup[16];
    printf("\nEnter IP address to lookup (eg: 192.168.2.100): ");
    scanf("%s", ip_to_lookup);

    // // Pre-determined input
    // // Insert prefixes (dotted-decimal IP, prefix length, next hop)
    // insert_prefix("192.168.1.0", 24, 1);
    // insert_prefix("192.168.2.0", 24, 2);
    // insert_prefix("192.168.0.0", 16, 3);  // Shorter prefix

    // // Lookup example IP
    // const char *ip_to_lookup = "192.168.2.100";  // Dotted-decimal IP

    unsigned int answer = ip_lookup(ip_to_lookup);
    if(answer == -1) {
        printf("\n------No prefix match found------\n\n");
        return 1;
    }

    printf("\nLongest prefix match for IP %s is: ", ip_to_lookup);
    print_ip(answer);
    printf("\n");

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
