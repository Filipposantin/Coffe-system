/** @file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "order_system.h"

/**
 * Reads the file of the name provided.
 * @param fptr pointer to the FILE object
 * @param name File name
 */


void read_file(FILE **fptr, char* name) {
    if ((*fptr = fopen(name, "r")) == NULL) {
        printf("Error! opening file %s\n", name);
        exit(0);
        // Program exits if file pointer returns NULL.
    } else {
        printf("File %s opened successfully\n", name);
    }
}

/**
 * Reads system information from the file into the SystemInfo struct
 * @param fptr Pointer to file
 * @param system_info Pointer to SystemInfo struct.
 */
void extract_system_info(FILE *fptr, struct SystemInfo *system_info) {
    fscanf(fptr, "%d %d\n", &system_info->storage_capacity, &system_info->number_of_workers);
}

/**
 * Reads models information from the file into ModelInfo structs.
 * @param fptr Pointer to file
 * @param models Pointer to array of ModelInfo structs.
 */
void extract_models_info(FILE *fptr, struct ModelInfo *models) {
    int i;
    for(i = 0 ; i < TOTAL_MODELS ; ++i) {
        fscanf(fptr, "%c %f %f %d %d\n", &models[i].model, &models[i].cost, &models[i].price, &models[i].space_required, &models[i].man_hours);
    }
}

/**
 * Reads customer orders from file into list of Order structs
 * @param fptr Pointer to file orders.dat
 * @param orders Pointer to array of Order structs.
 * @param orders_count Number of MAX_ORDERS that can be received.
 * @param stats Pointer to ModelOrderingStats struct.
 * @return Returns the number of orders read from the file.
 */
int extract_orders_info(FILE *fptr, struct Order *orders, int orders_count, struct ModelOrderingStats *stats) {
    int i = 0;
    while(fscanf(fptr,"%d %c %d %s\n", &orders[i].timestamp,&orders[i].model, &orders[i].quantity, orders[i].customer) != EOF) {
        update_ordering_stats(orders[i].model, stats);
        i++;
        if(orders_count == i) {
            return i;
        }
    }
    return i;
}

/**
 * Updates the number of orders placed by customer according to the item model provided.
 * @param model_name Name of the item model.
 * @param orderingStats Pointer reference to the ModelOrderingStats data structure
 */
void update_ordering_stats(char model_name, struct ModelOrderingStats *orderingStats) {
  switch (model_name) {
  case 'A':
    orderingStats->model_a_orders++;
    break;
  case 'B':
    orderingStats->model_b_orders++;
    break;
  case 'C':
    orderingStats->model_c_orders++;
    break;
  case 'D':
    orderingStats->model_d_orders++;
  default:
    break;
  }
}


/**
 * Gives reference to the model struct based on model name provided
 * @param name Item model
 * @param models Pointer to the array of ModelInfo structs.
 * @return Returns reference to the ModelInfo struct associated with the model provided.
 */
struct ModelInfo* get_model_by_name(char name, struct ModelInfo *models) {
    int i;
    for(i = 0 ; i < TOTAL_MODELS ; ++i) {
        if (models[i].model == name) {
            return &models[i];
        }
    }

    return NULL;
}

/**
 * Sorts orders by priority based on quantity of the item models as well as margin associated with each item model/
 * @param orders Pointer to the array of Order structs.
 * @param models Pointer to the array of ModelInfo structs.
 * @param orders_count Number of orders received by customers.
 */
void sort_by_priority(struct Order *orders, struct ModelInfo *models, int orders_count) {
    int i, j;
    struct Order temp;
    struct ModelInfo *order_i_model_ptr, *order_j_model_ptr;
    for(i = 0; i < orders_count; ++i) {

        for (j = i + 1; j < orders_count; ++j) {
            if (orders[i].timestamp == orders[j].timestamp) {
                if (orders[i].quantity < orders[j].quantity) {

                    temp = orders[i];
                    orders[i] = orders[j];
                    orders[j] = temp;

                } else if (orders[j].quantity == orders[i].quantity) {
                    order_i_model_ptr = get_model_by_name(orders[i].model, models);
                    order_j_model_ptr = get_model_by_name(orders[j].model, models);

                    if ((order_i_model_ptr->price - order_i_model_ptr->cost) <
                        (order_j_model_ptr->price - order_j_model_ptr->cost)) {

                        temp = orders[i];
                        orders[i] = orders[j];
                        orders[j] = temp;

                    }
                }

            }
        }

    }
}


/**
 * Prepares items to be stored in the stock according to the sales percentage.
 * @param stats Pointer reference to the ModelOrderingStats struct.
 * @param system Pointer reference to the SystemInfo struct.
 * @param stock  Pointer reference to the stock struct.
 * @param models Pointer reference to the array of ModelInfo struct.
 */

void prepare_for_stock(struct ModelOrderingStats *stats, struct SystemInfo *system, struct Stock *stock, struct ModelInfo *models) {
    int i, j = 0;
    int available_space = (system->storage_capacity - stock->occupied_space)/system->average_product_size;
    int model_a_stocks = available_space * ((float)stats->model_a_orders/stats->total_orders*100) / 100;
    int model_b_stocks = available_space * ((float)stats->model_b_orders/stats->total_orders*100) / 100;
    int model_c_stocks = available_space * ((float)stats->model_c_orders/stats->total_orders*100) / 100;
    int model_d_stocks = available_space * ((float)stats->model_d_orders/stats->total_orders*100) / 100;

    j = prepare_product_for_model('A', models, stock, system, model_a_stocks, j);
    j = prepare_product_for_model('B', models, stock, system, model_b_stocks, j);
    j = prepare_product_for_model('C', models, stock, system, model_c_stocks, j);
    j = prepare_product_for_model('D', models, stock, system, model_d_stocks, j);
}

/**
 * Manufacturers the product of the given model and adds it to the stock.
 * @param model Item model
 * @param models Pointer reference to the array of ModelInfo data structure
 * @param stock Pointer reference to the Stock struct.
 * @param system Pointer reference to the SystemInfo struct.
 * @param product_to_prepare Number of products to prepare for this specific model.
 * @param stock_index Current slot of the stock from where the items should be added.
 * @return Returns the new index of the stock after adding items for this model.
 */
int prepare_product_for_model(char model, struct ModelInfo *models, struct Stock *stock, struct SystemInfo *system, int product_to_prepare, int stock_index) {
    int i;
    struct ModelInfo *model_info = get_model_by_name(model, models);
    stock->products[stock_index].model = model;

    for(i = 0 ; i < product_to_prepare; ++i) {
        if (system->storage_capacity - model_info->space_required < 0) {
            return stock_index;
        }

        stock->products[stock_index].quantity++;
        stock->occupied_space += model_info->space_required;
        stock_index++;
    }
    return stock_index;
}

/**
 * Gives the average size of the item for storing in the stock based on all the item models.
 * @param models Pointer reference to the array of ModelInfo structs.
 * @return Returns average size of item.
 */
int average_product_size(struct ModelInfo *models) {
  int i, products_space_sum = 0;
  
  for(i = 0 ; i < TOTAL_MODELS; ++i) {
    products_space_sum += models[i].space_required;
  }
  
  return products_space_sum/TOTAL_MODELS;
}

/**
 * Processes the customer orders by using the already manufacturing items as well as making new items at spot.
 * @param orders Pointer reference to the array of Order structs.
 * @param stats Pointer reference to the ModelOrderingStats struct.
 * @param system Pointer reference to the SystemInfo struct.
 * @param stock Pointer reference to the Stock struct.
 * @param models Pointer reference to the array of ModelInfo data structure
 */
void process_orders(struct Order *orders, struct ModelOrderingStats *stats, struct SystemInfo *system, struct Stock *stock, struct ModelInfo *models) {
    int i, k;
    int processed_orders = 0;
    bool stock_prepared_for_free_days = false;
    int hour_count = 1;
    struct Order last_order;

    while (processed_orders < stats->total_orders) {
        for(i=0; i < stats->total_orders ; ++i) {
            update_ordering_stats(orders[i].model, stats);

            if (i == 0) {
                last_order = orders[0];
            }
            else {
                last_order = orders[i - 1];
            }

            struct ModelInfo *model_info = get_model_by_name(orders[i].model, models);

            if (!stock_prepared_for_free_days && orders[i].timestamp  - last_order.timestamp > 1) {
                for (k = last_order.timestamp + 1; k < orders[i].timestamp; ++k) {
                    printf("No orders placed on %d\n", k);
                    printf("Prepare models for storing in stock\n");
                    prepare_for_stock(stats, system, stock, models);
                }
            }

            if (sold_item_from_stock(orders[i].model, stock)) {
                continue;
            }

            if (orders[i].processing) {

                if (orders[i].process_end_hour == hour_count) {
                    orders[i].completed = 1;
                    orders[i].processing = 0;
                    system->number_of_workers += model_info->man_hours;
                    processed_orders++;
                    printf("Completed Order of %d items of  Model %c by %s at %d\n",orders[i].quantity, orders[i].model, orders[i].customer,  hour_count);
                }
            }
            else if (!orders[i].completed) {
                if (model_info->man_hours <= system->number_of_workers) {
                    system->number_of_workers -= model_info->man_hours;
                    orders[i].process_start_hour = hour_count;
                    orders[i].process_end_hour = hour_count + model_info->man_hours;
                    orders[i].processing = 1;
                    printf("Started processing order of %d items of  Model %c by %s at %d\n",orders[i].quantity, orders[i].model, orders[i].customer,  hour_count);
                }
                else {
                    printf("Not enough workers available, waiting");
                }
            }
        }
        stock_prepared_for_free_days = true;
        hour_count++;
    }
}


/**
 * Sells already made item from stock
 * @param model Item model
 * @param stock Pointer reference to Stock struct
 * @return True if item sold, otherwise false.
 */

bool sold_item_from_stock(char model, struct Stock *stock) {
    int i;

    for(i = 0 ; i < stock->products_index; ++i) {
        if (stock->products[i].model == model) {
            stock->products_index--;
            return true;
        }
    }

    return false;
}

/**
 * Finds the ItemSoldStats object associated with the provided customer name.
 * @param name Customer name
 * @param stats Pointer reference to array of ItemSoldStats struct.
 * @param stats_count Total number of stats identified.
 * @return Pointer reference to the object associated with provided customer name.
 */
struct ItemSoldStats * get_stats_from_customer_name(char * name, struct ItemSoldStats *stats, int stats_count) {
    int i;

    for(i = 0 ; i < stats_count ; ++i) {
        if ( strcmp(stats[i].name, name) == 0 ) {
            return &stats[i];
        }
    }
    return NULL;
}


/**
 * Calculates the amount of each item models sold to each customer.
 * @param orders Pointer reference to array of Order structs.
 * @param itemSoldStats Pointer reference to ItemSoldStats struct.
 * @param orders_count Total orders placed by customer.
 * @return Returns the total number of unique customers to whom the items were sold.
 */

int calculate_items_sold_for_each_customer(struct Order *orders, struct ItemSoldStats *itemSoldStats, int orders_count) {
    int i, j = 0;

    for(i = 0 ; i < orders_count ; ++i) {
        struct ItemSoldStats *stat = get_stats_from_customer_name(orders[i].customer, itemSoldStats, j);

        if (stat == NULL) {
            stat = &itemSoldStats[j];
            strcpy(itemSoldStats[j].name, orders[i].customer);
            j++;
        }
        switch (orders[i].model) {
            case 'A':
                stat->model_a_products++;
                break;
            case 'B':
                stat->model_b_products++;
                break;
            case 'C':
                stat->model_c_products++;
                break;
            case 'D':
                stat->model_d_products++;
                break;
        }
    }

    return j;
}

/**
 * Sorts by most recent orders
 * @param orders Pointer referece to array of Order struct.
 * @param orders_count Total orders placed by customers.
 */

void sort_by_day(struct Order *orders, int orders_count) {
    int i, j ;

    struct Order temp;

    for(i = 0; i < orders_count; ++i) {
        for (j = i + 1; j < orders_count; ++j) {
            if (orders[i].timestamp > orders[j].timestamp) {
                temp = orders[i];
                orders[i] = orders[j];
                orders[j] = temp;
            }
        }
    }
}

/**
 * Calculate the stats over the last 12 months
 * @param orders Pointer reference to the array of Order structs.
 * @param models Pointer reference to the array of ModelInfo structs/
 * @param stats Pointer reference to the TwelveMonthStats struct.
 * @param orders_count Total number of orders placed by customers.
 */
void calculate_twelve_month_stats(struct Order *orders, struct ModelInfo *models, struct TwelveMonthStats *stats, int orders_count) {
  int i;
  int end_day = orders[orders_count - 1].timestamp - 365;
  
  for( i = orders_count - 1 ; i >= 0 ; --i) {
    if (orders[i].timestamp < end_day) {
      break;
    }
    struct ModelInfo *model_info = get_model_by_name(orders[i].model, models);
    stats->revenue += model_info->price * orders[i].quantity;
    stats->margin += (model_info->price - model_info->cost) * orders[i].quantity;
  }
}


/**
 * Main entry point of the program.
 * @return Return 0 if programs executed successfully.
 */
int main() {
    int i, total_orders = 0;
    FILE *info_file_ptr, *orders_file_ptr;
    struct SystemInfo system;
    struct ModelInfo models[TOTAL_MODELS];
    struct ModelOrderingStats ordering_stats;
    struct Order orders[20];
    struct ItemSoldStats item_sold_stats[20];
    struct TwelveMonthStats twelve_month_stats;
    struct Stock stock;

    read_file(&info_file_ptr, "info.dat");
    extract_system_info(info_file_ptr, &system);
    extract_models_info(info_file_ptr, models);

    read_file(&orders_file_ptr, "orders.dat");
    ordering_stats.total_orders = extract_orders_info(orders_file_ptr, orders, 100, &ordering_stats);

    system.average_product_size = average_product_size(models);

    printf("Orders found : %d\n", ordering_stats.total_orders);
    
    sort_by_priority(orders, models, ordering_stats.total_orders);

    printf("%15s %15s  %15s  %15s\n", "Customer", "Quantity", "Model", "Timestamp");
    for(i = 0 ; i < ordering_stats.total_orders; ++i) {
        printf("%14s  %14d  %14c  %14d\n", orders[i].customer, orders[i].quantity, orders[i].model, orders[i].timestamp);
    }
    
    process_orders(orders, &ordering_stats, &system, &stock, models);

    int total_customers = calculate_items_sold_for_each_customer(orders, item_sold_stats, ordering_stats.total_orders);
    
    sort_by_day(orders, ordering_stats.total_orders);
    
    calculate_twelve_month_stats(orders, models, &twelve_month_stats, ordering_stats.total_orders);

    printf("====================================\n");
    printf("======= Sold Items Statistics ======\n");
    printf("====================================\n");
    for(i=0 ; i < total_customers ; ++i) {
        printf("Customer name: %s\n", item_sold_stats[i].name);
        printf("Model A items sold: %d\n", item_sold_stats[i].model_a_products);
        printf("Model B items sold: %d\n", item_sold_stats[i].model_b_products);
        printf("Model C items sold: %d\n", item_sold_stats[i].model_c_products);
        printf("Model D items sold: %d\n", item_sold_stats[i].model_d_products);
    }
    

    printf("============================================\n");
    printf("======= Last Twelve Months Statistics ======\n");
    printf("============================================\n");

    printf("Margin : %d euro\n", twelve_month_stats.margin);
    printf("Revenue: %d euro\n", twelve_month_stats.revenue);

    fclose(info_file_ptr);
    fclose(orders_file_ptr);
    free(info_file_ptr);
    free(orders_file_ptr);
    return 0;
}
