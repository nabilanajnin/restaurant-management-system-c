#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MENU 100
#define MAX_LINES 10
#define NAME_LEN 40
#define STATUS_LEN 12
#define BUF_LEN 128

struct MenuItem
{
    int id;
    char name[NAME_LEN];
    float price;
};

struct OrderLine
{
    int itemId;
    char itemName[NAME_LEN];
    int quantity;
    float unitPrice;
};

struct Order
{
    int orderId;
    char customerName[NAME_LEN];
    struct OrderLine lines[MAX_LINES];
    int lineCount;
    float totalBill;
    char status[STATUS_LEN];
    struct Order *next;
};

struct QNode //used for kitchen queue
{
    struct Order *order;
    struct QNode *next;
};

struct SNode //used for storing the completed/canceled order using stack
{
    struct Order *order;
    struct SNode *next;
};

struct MenuItem menu[MAX_MENU];
int menuCount = 0;
int nextItemId = 1;
int nextOrderId = 1001;

struct Order *activeHead = NULL;
struct QNode *queueFront = NULL;
struct QNode *queueRear = NULL;
struct SNode *completedTop = NULL;
struct SNode *cancelledTop = NULL;

int readLine(char *buffer, int size);
int readInt(const char *prompt, int *out);
int readFloat(const char *prompt, float *out);
void readText(const char *prompt, char *out, int size);
void pauseScreen(void);
void printRule(void);

void displayMainMenu(void);
void customerMenu(void);
void kitchenMenu(void);
void adminMenu(void);
void reportMenu(void);

void seedMenu(void);
void addMenuItem(void);
void deleteMenuItem(void);
void updateMenuItem(void);
void displayMenu(void);
int searchMenuItem(int id);
void bubbleSortMenu(void);
void swapMenuItems(int a, int b);

void placeOrder(void);
void searchOrderMenu(void);
void cancelOrder(void);
struct Order *searchOrder(int orderId);
void insertActiveOrder(struct Order *order);
struct Order *removeActiveOrder(int orderId);
void printOrder(const struct Order *order);

void enqueueOrder(struct Order *order);
struct Order *dequeueOrder(void);
int removeFromQueue(int orderId);
int queueSize(void);

void pushCompleted(struct Order *order);
void pushCancelled(struct Order *order);
void displayCompleted(void);
void displayCancelled(void);

void prepareNextOrder(void);

float calculateTotalSales(void);
int countStack(struct SNode *node);
int countActive(void);

void freeAll(void);

int readLine(char *buffer, int size)
{
    if (fgets(buffer, size, stdin) == NULL)
    {
        return 0;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    return 1;
}

int readInt(const char *prompt, int *out)
{
    char buffer[BUF_LEN];
    char extra;

    printf("%s", prompt);
    if (!readLine(buffer, BUF_LEN))
    {
        return 0;
    }

    if (sscanf(buffer, "%d %c", out, &extra) != 1)
    {
        printf("  ! Please enter a whole number.\n");
        return 0;
    }
    return 1;
}

int readFloat(const char *prompt, float *out)
{
    char buffer[BUF_LEN];
    char extra;

    printf("%s", prompt);
    if (!readLine(buffer, BUF_LEN))
    {
        return 0;
    }
    if (sscanf(buffer, "%f %c", out, &extra) != 1)
    {
        printf("  ! Please enter a number.\n");
        return 0;
    }
    return 1;
}

void readText(const char *prompt, char *out, int size)
{
    while (1)
    {
        printf("%s", prompt);
        if (!readLine(out, size))
        {
            out[0] = '\0';
            return;
        }
        if (out[0] != '\0')
        {
            return;
        }
        printf("  ! This field cannot be empty.\n");
    }
}

void pauseScreen(void)
{
    char buffer[BUF_LEN];
    printf("\nPress Enter to continue...");
    readLine(buffer, BUF_LEN);
}

void printRule(void)
{
    printf("--------------------------------------------------\n");
}

void displayMainMenu(void)
{
    printf("\n");
    printRule();
    printf("       RESTAURANT MANAGEMENT SYSTEM\n");
    printRule();
    printf("  1. Customer\n");
    printf("  2. Kitchen\n");
    printf("  3. Admin\n");
    printf("  4. Exit\n");
    printRule();
}

int main(void)
{
    int choice;

    seedMenu();

    while (1)
    {
        displayMainMenu();
        if (!readInt("Choose an option: ", &choice))
        {
            continue;
        }
        switch (choice)
        {
        case 1:
            customerMenu();
            break;
        case 2:
            kitchenMenu();
            break;
        case 3:
            adminMenu();
            break;
        case 4:
            printf("\nExiting. Goodbye!\n");
            freeAll();
            return 0;
        default:
            printf("  ! Choose a number between 1 and 4.\n");
        }
    }
}

void seedMenu(void)
{
    static const char *names[] = {"Chicken Burger", "Beef Steak", "Cheese Pizza","Veg Salad", "Cold Coffee", "Vanilla Ice Cream"};
    static const float prices[] = {5.50f, 12.75f, 8.00f, 3.25f, 2.50f, 2.00f};
    int i;

    for (i = 0; i < 6; i++)
    {
        menu[menuCount].id = nextItemId++;
        strncpy(menu[menuCount].name, names[i], NAME_LEN - 1);
        menu[menuCount].name[NAME_LEN - 1] = '\0';
        menu[menuCount].price = prices[i];
        menuCount++;
    }
}

void addMenuItem(void)
{
    char name[NAME_LEN];
    float price;

    if (menuCount >= MAX_MENU)
    {
        printf("  ! Menu is full (%d items).\n", MAX_MENU);
        return;
    }
    readText("Item name  : ", name, NAME_LEN);
    if (!readFloat("Item price : ", &price))
    {
        return;
    }
    if (price <= 0.0f)
    {
        printf("  ! Price must be greater than zero.\n");
        return;
    }

    menu[menuCount].id = nextItemId++;
    strncpy(menu[menuCount].name, name, NAME_LEN - 1);
    menu[menuCount].name[NAME_LEN - 1] = '\0';
    menu[menuCount].price = price;
    menuCount++;

    printf("  > Added item #%d.\n", menu[menuCount - 1].id);
}

void deleteMenuItem(void)
{
    int id, index, i;

    if (menuCount == 0)
    {
        printf("  ! The menu is empty.\n");
        return;
    }
    displayMenu();
    if (!readInt("ID of the item to delete: ", &id))
    {
        return;
    }
    index = searchMenuItem(id);
    if (index == -1)
    {
        printf("  ! No menu item with ID %d.\n", id);
        return;
    }

    printf("  > Deleted \"%s\".\n", menu[index].name);

    for (i = index; i < menuCount - 1; i++)
    {
        menu[i] = menu[i + 1];
    }
    menuCount--;
}

void updateMenuItem(void)
{
    int id, index;
    char name[NAME_LEN];
    float price;

    if (menuCount == 0)
    {
        printf("  ! The menu is empty.\n");
        return;
    }
    displayMenu();
    if (!readInt("ID of the item to update: ", &id))
    {
        return;
    }
    index = searchMenuItem(id);
    if (index == -1)
    {
        printf("  ! No menu item with ID %d.\n", id);
        return;
    }

    printf("Current: %s - %.2f\n", menu[index].name, menu[index].price);
    readText("New name  : ", name, NAME_LEN);
    if (!readFloat("New price : ", &price))
    {
        return;
    }
    if (price <= 0.0f)
    {
        printf("  ! Price must be greater than zero.\n");
        return;
    }

    strncpy(menu[index].name, name, NAME_LEN - 1);
    menu[index].name[NAME_LEN - 1] = '\0';
    menu[index].price = price;
    printf("  > Item #%d updated.\n", id);
}

void displayMenu(void)
{
    int i;

    printf("\n");
    printRule();
    printf(" %-5s %-30s %10s\n", "ID", "ITEM", "PRICE");
    printRule();
    if (menuCount == 0)
    {
        printf(" (the menu is empty)\n");
    }
    for (i = 0; i < menuCount; i++)
    {
        printf(" %-5d %-30s %10.2f\n", menu[i].id, menu[i].name, menu[i].price);
    }
    printRule();
    printf(" %d item(s)\n", menuCount);
}

void swapMenuItems(int a, int b)
{
    struct MenuItem temp = menu[a];
    menu[a] = menu[b];
    menu[b] = temp;
}

void bubbleSortMenu(void)
{
    int i, j, swapped;

    for (i = 0; i < menuCount - 1; i++)
    {
        swapped = 0;
        for (j = 0; j < menuCount - 1 - i; j++)
        {
            if (menu[j].price > menu[j + 1].price)
            {
                swapMenuItems(j, j + 1);
                swapped = 1;
            }
        }
        if (!swapped)
        {
            break;
        }
    }
}

void adminMenu(void)
{
    int choice;

    while (1)
    {
        printf("\n");
        printRule();
        printf("       ADMIN MENU\n");
        printRule();
        printf("  1. Add Menu Item\n");
        printf("  2. Delete Menu Item\n");
        printf("  3. Update Menu Item\n");
        printf("  4. Display Menu\n");
        printf("  5. Sort Menu by Price (Bubble Sort)\n");
        printf("  6. Reports\n");
        printf("  7. Back\n");
        printRule();

        if (!readInt("Choose an option: ", &choice))
        {
            continue;
        }
        switch (choice)
        {
        case 1:
            addMenuItem();
            pauseScreen();
            break;
        case 2:
            deleteMenuItem();
            pauseScreen();
            break;
        case 3:
            updateMenuItem();
            pauseScreen();
            break;
        case 4:
            displayMenu();
            pauseScreen();
            break;
        case 5:
            bubbleSortMenu();
            printf("  > Menu sorted by price.\n");
            displayMenu();
            pauseScreen();
            break;
        case 6:
            reportMenu();
            break;
        case 7:
            return;
        default:
            printf("  ! Choose a number between 1 and 7.\n");
        }
    }
}

int searchMenuItem(int id)
{
    int i;

    for (i = 0; i < menuCount; i++)
    {
        if (menu[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

struct Order *searchOrder(int orderId)
{
    struct Order *current;

    for (current = activeHead; current != NULL; current = current->next)
    {
        if (current->orderId == orderId)
        {
            return current;
        }
    }
    return NULL;
}

void insertActiveOrder(struct Order *order)
{
    struct Order *current;

    order->next = NULL;
    if (activeHead == NULL)
    {
        activeHead = order;
        return;
    }
    current = activeHead;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = order;
}

struct Order *removeActiveOrder(int orderId)
{
    struct Order *current = activeHead;
    struct Order *previous = NULL;

    while (current != NULL && current->orderId != orderId)
    {
        previous = current;
        current = current->next;
    }
    if (current == NULL)
    {
        return NULL;
    }
    if (previous == NULL)
    {
        activeHead = current->next;
    }
    else
    {
        previous->next = current->next;
    }
    current->next = NULL;
    return current;
}

void printOrder(const struct Order *order)
{
    int i;

    printRule();
    printf(" Order #%d   Customer: %s   [%s]\n",
           order->orderId, order->customerName, order->status);
    printRule();
    for (i = 0; i < order->lineCount; i++)
    {
        printf("   %-25s %3d x %8.2f = %8.2f\n",
               order->lines[i].itemName,
               order->lines[i].quantity,
               order->lines[i].unitPrice,
               order->lines[i].quantity * order->lines[i].unitPrice);
    }
    printf("   %-25s %25.2f\n", "TOTAL", order->totalBill);
    printRule();
}

void enqueueOrder(struct Order *order)
{
    struct QNode *node = (struct QNode *)malloc(sizeof(struct QNode));

    if (node == NULL)
    {
        printf("  ! Out of memory - order not queued.\n");
        return;
    }
    node->order = order;
    node->next = NULL;

    if (queueRear == NULL)
    {
        queueFront = node;
        queueRear = node;
    }
    else
    {
        queueRear->next = node;
        queueRear = node;
    }
}

struct Order *dequeueOrder(void)
{
    struct QNode *node;
    struct Order *order;

    if (queueFront == NULL)
    {
        return NULL;
    }
    node = queueFront;
    order = node->order;

    queueFront = node->next;
    if (queueFront == NULL)
    {
        queueRear = NULL;
    }
    free(node);
    return order;
}

int removeFromQueue(int orderId)
{
    struct QNode *current = queueFront;
    struct QNode *previous = NULL;

    while (current != NULL && current->order->orderId != orderId)
    {
        previous = current;
        current = current->next;
    }
    if (current == NULL)
    {
        return 0;
    }
    if (previous == NULL)
    {
        queueFront = current->next;
    }
    else
    {
        previous->next = current->next;
    }
    if (current == queueRear)
    {
        queueRear = previous;
    }
    free(current);
    return 1;
}

int queueSize(void)
{
    struct QNode *current;
    int count = 0;

    for (current = queueFront; current != NULL; current = current->next)
    {
        count++;
    }
    return count;
}

static void pushStack(struct SNode **top, struct Order *order)
{
    struct SNode *node = (struct SNode *)malloc(sizeof(struct SNode));

    if (node == NULL)
    {
        printf("  ! Out of memory - order not archived.\n");
        return;
    }
    node->order = order;
    node->next = *top;
    *top = node;
}

void pushCompleted(struct Order *order)
{
    strcpy(order->status, "Completed");
    pushStack(&completedTop, order);
}

void pushCancelled(struct Order *order)
{
    strcpy(order->status, "Cancelled");
    pushStack(&cancelledTop, order);
}

void displayCompleted(void)
{
    struct SNode *current;

    printf("\n=== COMPLETED ORDERS (newest first) ===\n");
    if (completedTop == NULL)
    {
        printf(" (none yet)\n");
    }
    for (current = completedTop; current != NULL; current = current->next)
    {
        printOrder(current->order);
    }
}

void displayCancelled(void)
{
    struct SNode *current;

    printf("\n=== CANCELLED ORDERS (newest first) ===\n");
    if (cancelledTop == NULL)
    {
        printf(" (none yet)\n");
    }
    for (current = cancelledTop; current != NULL; current = current->next)
    {
        printOrder(current->order);
    }
}

void placeOrder(void)
{
    struct Order *order;
    int id, index, quantity;
    char more[BUF_LEN];

    if (menuCount == 0)
    {
        printf("  ! Nothing on the menu yet - ask the admin to add items.\n");
        return;
    }

    order = (struct Order *)malloc(sizeof(struct Order));
    if (order == NULL)
    {
        printf("  ! Out of memory - could not create the order.\n");
        return;
    }
    order->lineCount = 0;
    order->totalBill = 0.0f;
    order->next = NULL;
    readText("Customer name: ", order->customerName, NAME_LEN);

    while (order->lineCount < MAX_LINES)
    {
        displayMenu();
        if (!readInt("Item ID (0 to finish): ", &id))
        {
            continue;
        }
        if (id == 0)
        {
            break;
        }
        index = searchMenuItem(id);
        if (index == -1)
        {
            printf("  ! No menu item with ID %d.\n", id);
            continue;
        }
        if (!readInt("Quantity: ", &quantity))
        {
            continue;
        }
        if (quantity <= 0 || quantity > 100)
        {
            printf("  ! Quantity must be between 1 and 100.\n");
            continue;
        }

        order->lines[order->lineCount].itemId = menu[index].id;
        strncpy(order->lines[order->lineCount].itemName, menu[index].name, NAME_LEN - 1);
        order->lines[order->lineCount].itemName[NAME_LEN - 1] = '\0';
        order->lines[order->lineCount].quantity = quantity;
        order->lines[order->lineCount].unitPrice = menu[index].price;
        order->totalBill += quantity * menu[index].price;
        order->lineCount++;

        printf("  > Added %d x %s.\n", quantity, menu[index].name);
        if (order->lineCount == MAX_LINES)
        {
            printf("  ! Order limit of %d different items reached.\n", MAX_LINES);
            break;
        }
        printf("Add another item? (y/n): ");
        readLine(more, BUF_LEN);
        if (more[0] != 'y' && more[0] != 'Y')
        {
            break;
        }
    }

    if (order->lineCount == 0)
    {
        printf("  ! Empty order discarded.\n");
        free(order);
        return;
    }

    order->orderId = nextOrderId++;
    strcpy(order->status, "Waiting");
    insertActiveOrder(order);
    enqueueOrder(order);

    printf("\n  > Order placed and sent to the kitchen.\n");
    printOrder(order);
}

void searchOrderMenu(void)
{
    int orderId;
    struct Order *order;

    if (!readInt("Order ID: ", &orderId))
    {
        return;
    }
    order = searchOrder(orderId);
    if (order == NULL)
    {
        printf("  ! Order #%d is not active (it may be completed or cancelled).\n",
               orderId);
        return;
    }
    printOrder(order);
}

void cancelOrder(void)
{
    int orderId;
    struct Order *order;

    if (activeHead == NULL)
    {
        printf("  ! There are no active orders.\n");
        return;
    }
    if (!readInt("Order ID to cancel: ", &orderId))
    {
        return;
    }
    if (searchOrder(orderId) == NULL)
    {
        printf("  ! Order #%d is not active.\n", orderId);
        return;
    }

    removeFromQueue(orderId);
    order = removeActiveOrder(orderId);
    pushCancelled(order);
    printf("  > Order #%d cancelled.\n", orderId);
}

void customerMenu(void)
{
    int choice;

    while (1)
    {
        printf("\n");
        printRule();
        printf("       CUSTOMER MENU\n");
        printRule();
        printf("  1. View Menu\n");
        printf("  2. Place Order\n");
        printf("  3. Search Order\n");
        printf("  4. Cancel Order\n");
        printf("  5. Back\n");
        printRule();

        if (!readInt("Choose an option: ", &choice))
        {
            continue;
        }
        switch (choice)
        {
        case 1:
            displayMenu();
            pauseScreen();
            break;
        case 2:
            placeOrder();
            pauseScreen();
            break;
        case 3:
            searchOrderMenu();
            pauseScreen();
            break;
        case 4:
            cancelOrder();
            pauseScreen();
            break;
        case 5:
            return;
        default:
            printf("  ! Choose a number between 1 and 5.\n");
        }
    }
}

void prepareNextOrder(void)
{
    struct Order *queued;
    struct Order *order;

    queued = dequeueOrder();
    if (queued == NULL)
    {
        printf("  ! The kitchen queue is empty.\n");
        return;
    }

    order = removeActiveOrder(queued->orderId);
    if (order == NULL)
    {
        return;
    }
    pushCompleted(order);
    printf("  > Order #%d prepared and served.\n", order->orderId);
    printOrder(order);
}

void kitchenMenu(void)
{
    int choice;

    while (1)
    {
        printf("\n");
        printRule();
        printf("       KITCHEN MENU\n");
        printRule();
        printf("  1. View Waiting Orders\n");
        printf("  2. Prepare Next Order\n");
        printf("  3. Back\n");
        printRule();

        if (!readInt("Choose an option: ", &choice))
        {
            continue;
        }
        switch (choice)
        {
        case 1:
            printf("\n%d order(s) waiting to be prepared.\n", queueSize());
            if (queueFront != NULL)
            {
                printf("Next up: Order #%d for %s.\n",
                       queueFront->order->orderId,
                       queueFront->order->customerName);
            }
            pauseScreen();
            break;
        case 2:
            prepareNextOrder();
            pauseScreen();
            break;
        case 3:
            return;
        default:
            printf("  ! Choose a number between 1 and 3.\n");
        }
    }
}

float calculateTotalSales(void)
{
    struct SNode *current;
    float total = 0.0f;

    for (current = completedTop; current != NULL; current = current->next)
    {
        total += current->order->totalBill;
    }
    return total;
}

int countStack(struct SNode *node)
{
    if (node == NULL)
    {
        return 0;
    }
    return 1 + countStack(node->next);
}

int countActive(void)
{
    struct Order *current;
    int count = 0;

    for (current = activeHead; current != NULL; current = current->next)
    {
        count++;
    }
    return count;
}

void reportMenu(void)
{
    int choice;

    while (1)
    {
        printf("\n");
        printRule();
        printf("       REPORTS MENU\n");
        printRule();
        printf("  1. View Completed Orders\n");
        printf("  2. View Cancelled Orders\n");
        printf("  3. Total Sales\n");
        printf("  4. Total Orders\n");
        printf("  5. Back\n");
        printRule();

        if (!readInt("Choose an option: ", &choice))
        {
            continue;
        }
        switch (choice)
        {
        case 1:
            displayCompleted();
            pauseScreen();
            break;
        case 2:
            displayCancelled();
            pauseScreen();
            break;
        case 3:
            printf("\nTotal sales from %d completed order(s): %.2f\n",
                   countStack(completedTop), calculateTotalSales());
            pauseScreen();
            break;
        case 4:
            printf("\n");
            printRule();
            printf(" Active (waiting) : %d\n", countActive());
            printf(" Completed        : %d\n", countStack(completedTop));
            printf(" Cancelled        : %d\n", countStack(cancelledTop));
            printRule();
            printf(" Total            : %d\n",
                   countActive() + countStack(completedTop) + countStack(cancelledTop));
            pauseScreen();
            break;
        case 5:
            return;
        default:
            printf("  ! Choose a number between 1 and 5.\n");
        }
    }
}

static void freeStack(struct SNode *top)
{
    struct SNode *next;

    while (top != NULL)
    {
        next = top->next;
        free(top->order);
        free(top);
        top = next;
    }
}

void freeAll(void)
{
    struct Order *order;
    struct Order *nextOrder;
    struct QNode *node;
    struct QNode *nextNode;

    node = queueFront;
    while (node != NULL)
    {
        nextNode = node->next;
        free(node);
        node = nextNode;
    }
    queueFront = NULL;
    queueRear = NULL;

    order = activeHead;
    while (order != NULL)
    {
        nextOrder = order->next;
        free(order);
        order = nextOrder;
    }
    activeHead = NULL;

    freeStack(completedTop);
    freeStack(cancelledTop);
    completedTop = NULL;
    cancelledTop = NULL;
}
