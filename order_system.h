#ifndef ORDER_SYSTEM_ORDER_SYSTEM_H
#define ORDER_SYSTEM_ORDER_SYSTEM_H
#define TOTAL_MODELS 4
#define MAX_PRODUCTS 100
#define MAX_ORDER_DAYS 5000
/** @file */

/**
 * Holds information for customer order including item model, item quantity, order time and customer name
 */
struct Order {
    int timestamp; /**< Time of the order*/
    char model; /**< Model of the item which can be A, B, C or D */
    int quantity; /**< Quantity of the item ordered */
    char customer[20]; /**< Customer name who ordered the item */
    int process_start_hour; /**< Order start hour */
    int process_end_hour; /**< Order end hour */
    int completed; /**< Order completed status */
    int processing; /** Order processing status */
};

/**
 * Holds information related to the coffee models which can be manufactured including their cost, price etc
 */
struct ModelInfo {
    char model; /**< Model of the item */
    float cost; /**< Cost for the manufacturing of the item model. */
    float price; /**< Sale price of the item model. */
    int man_hours; /**< Man hours required to manufacture the item model. */
    int space_required; /**< Space required to the stack the 1 item of this model. */
};

/**
 * Holds information regarding the system including stock capacity in m^3 etc.
 */
struct SystemInfo {
    int storage_capacity; /**< Stock capacity in m^3 */
    int number_of_workers; /**< Number of workers available */
    int average_product_size; /**< Average size of the product for storing the stock. */
    int days_without_orders[MAX_ORDER_DAYS]; /**< List of days on which there were no orders by customers */
    int days_without_orders_index; /**< Index of array of days without orders */
};

/**
 * Holds information regarding the orders placed by customers for different models as well as total orders placed.
 */
struct ModelOrderingStats {
    int model_a_orders; /**< Number of orders placed for Model A. */
    int model_b_orders; /**< Number of orders placed for Model B. */
    int model_c_orders; /**< Number of orders placed for Model C. */
    int model_d_orders; /**< Number of orders placed for Model D. */
    int total_orders; /**< Number of total orders. */
};

/**
 * Holds information for product which needs to be stored in stock.
 */
struct Product {
    char model; /**< Model of the item stored in stock. */
    int quantity; /**< Quantity of the item stored in stock. */
};

/**
 * Holds information for system stock.
 */
struct Stock {
    struct Product products[MAX_PRODUCTS]; /**< List of products currently stored in stock. */
    int occupied_space; /**< Total space occupied by all the items currently stored in stock. */
    int products_index; /**< Index of products list. */
};

/**
 * Holds information related to item models sold to each customer
 */
struct ItemSoldStats {
    char name[20]; /**< Customer name */
    int model_a_products; /**< Number of model A products sold to this customer. */
    int model_b_products; /**< Number of model B products sold to this customer. */
    int model_c_products; /**< Number of model C products sold to this customer. */
    int model_d_products; /**< Number of model D products sold to this customer. */
};

/**
 * Holds information of last twelve months
 */
struct TwelveMonthStats {
    int revenue; /**< Revenue over the last 12 months. */
    int margin; /**< Margin (sale - cost) over the last 12 months. */
};

void extract_system_info(FILE *, struct SystemInfo *);
void extract_models_info(FILE *, struct ModelInfo *);
int extract_orders_info(FILE *fptr, struct Order *, int, struct ModelOrderingStats *);
void update_ordering_stats(char, struct ModelOrderingStats *);
struct ModelInfo* get_model_by_name(char, struct ModelInfo *);
void sort_by_priority(struct Order *, struct ModelInfo *, int);
void prepare_for_stock(struct ModelOrderingStats *, struct SystemInfo *, struct Stock *, struct ModelInfo *);
int prepare_product_for_model(char, struct ModelInfo *, struct Stock *, struct SystemInfo *, int, int);
int average_product_size(struct ModelInfo *);
void process_orders(struct Order *, struct ModelOrderingStats *, struct SystemInfo *, struct Stock *, struct ModelInfo *);
bool sold_item_from_stock(char model, struct Stock *stock);
struct ItemSoldStats * get_stats_from_customer_name(char *, struct ItemSoldStats *, int);
int calculate_items_sold_for_each_customer(struct Order *, struct ItemSoldStats *, int );
void sort_by_day(struct Order *, int);
void calculate_twelve_month_stats(struct Order *, struct ModelInfo *, struct TwelveMonthStats *, int);

#endif //ORDER_SYSTEM_ORDER_SYSTEM_H
