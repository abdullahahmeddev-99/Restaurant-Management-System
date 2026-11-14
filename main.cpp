#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

const string ORDERS_FILE = "orders.dat";

enum class PaymentMethod { Cash = 1, CreditCard, Wallet };
enum class OrderType { DineIn = 1, Takeaway, Delivery };
enum class MealType { Pizza = 1, Burger, Potatoes, Chicken };

struct Order;

struct RestaurantData {
  double pizza_l = 250.00;
  double pizza_m = 150.00;
  double pizza_s = 100.00;
  double burger_beef = 200.00;
  double burger_chicken = 220.00;
  double potatoes = 120.00;
  double chicken = 600.00;

  int pizza_stock = 50;
  int burger_stock = 50;
  int potatoes_stock = 50;
  int chicken_stock = 30;

  double tax_rate = 0.14;
  double delivery_fee = 50.00;

  int total_orders = 0;
  int cash_orders = 0;
  int card_orders = 0;
  int wallet_orders = 0;

  double total_sales = 0.00;
  vector<Order> completed_orders;
};

struct RestaurantInfo {
  string name;
  string phone;
  int table_number = 0;
  bool discount_card = false;
  OrderType order_type = OrderType::Takeaway;
};

struct OrderItem {
  MealType type = MealType::Pizza;
  string name;
  string details;
  int quantity = 0;
  double unit_price = 0.00;
  double total_price = 0.00;
};

struct BillInfo {
  PaymentMethod payment = PaymentMethod::Cash;
  double paid_amount = 0.00;
};

struct Order {
  int order_id = 0;
  RestaurantInfo customer;
  OrderItem items[20];
  int item_count = 0;

  double subtotal = 0.00;
  double discount = 0.00;
  double tax = 0.00;
  double delivery_fee = 0.00;
  double final_total = 0.00;

  BillInfo bill;
};

int readInt(const string &msg, int min, int max) {
  string input;
  while (true) {
    cout << msg;
    getline(cin >> ws, input);

    stringstream ss(input);
    int n;
    char extra;

    if (ss >> n && !(ss >> extra) && n >= min && n <= max)
      return n;

    cout << "Invalid! Enter a number between " << min << " and " << max
         << ".\n";
  }
}

double readPositive(const string &msg) {
  string input;
  while (true) {
    cout << msg;
    getline(cin >> ws, input);

    stringstream ss(input);
    double n;
    char extra;

    if (ss >> n && !(ss >> extra) && n > 0)
      return n;

    cout << "Invalid! Enter a positive number.\n";
  }
}

bool validName(const string &name) {
  if (name.empty())
    return false;

  for (char c : name) {
    if (!isalpha(static_cast<unsigned char>(c)) && c != ' ')
      return false;
  }
  return true;
}

string readName() {
  string name;
  while (true) {
    cout << "Name: ";
    getline(cin >> ws, name);

    if (validName(name))
      return name;

    cout << "Invalid name! Use letters only (no numbers or symbols).\n";
  }
}

bool validPhone(const string &phone) {
  if (phone.length() != 11)
    return false;

  if (phone[0] != '0' || phone[1] != '1')
    return false;

  for (char c : phone) {
    if (!isdigit(static_cast<unsigned char>(c)))
      return false;
  }
  return true;
}

string readPhone() {
  string phone;
  while (true) {
    cout << "Phone: ";
    getline(cin >> ws, phone);

    if (validPhone(phone))
      return phone;

    cout << "Invalid phone! Enter an 11-digit number starting with 01.\n";
  }
}

bool yesNo(const string &msg) { return readInt(msg, 0, 1) == 1; }

void showMenu(const RestaurantData &d) {
  cout << "\n========== MENU ==========\n";
  cout << fixed << setprecision(2);

  cout << "1. Pizza\n";
  cout << "   Small: $" << d.pizza_s << " | Medium: $" << d.pizza_m
       << " | Large: $" << d.pizza_l << "\n";

  cout << "2. Burger\n";
  cout << "   Beef: $" << d.burger_beef << " | Chicken: $" << d.burger_chicken
       << "\n";

  cout << "3. Potatoes: $" << d.potatoes << "\n";

  cout << "4. Chicken: $" << d.chicken << " | Half: $" << d.chicken / 2
       << " | Quarter: $" << d.chicken / 4 << "\n";

  cout << "==========================\n";
}

void customerInfo(RestaurantInfo &c) {
  cout << "\n========== CUSTOMER ==========\n";

  c.name = readName();

  c.phone = readPhone();

  cout << "\n1. Dine-in\n";
  cout << "2. Takeaway\n";
  cout << "3. Delivery\n";

  c.order_type = static_cast<OrderType>(readInt("Choose order type: ", 1, 3));

  if (c.order_type == OrderType::DineIn)
    c.table_number = readInt("Table number: ", 1, 100);

  c.discount_card = yesNo("Discount card? (1 Yes / 0 No): ");

  cout << "Welcome " << c.name << "!\n";
}

int orderTypeToInt(OrderType t) { return static_cast<int>(t); }
OrderType intToOrderType(int t) { return static_cast<OrderType>(t); }
int mealTypeToInt(MealType t) { return static_cast<int>(t); }
MealType intToMealType(int t) { return static_cast<MealType>(t); }
int paymentMethodToInt(PaymentMethod p) { return static_cast<int>(p); }
PaymentMethod intToPaymentMethod(int p) { return static_cast<PaymentMethod>(p); }

int generateID() {
  static int id = 1000;
  static bool loaded = false;
  if (!loaded) {
    loaded = true;
    ifstream in(ORDERS_FILE);
    if (in) {
      string line;
      while (getline(in, line)) {
        if (line == "[ORDER]") {
          if (getline(in, line)) {
            int order_id = 0;
            try { order_id = stoi(line); } catch (...) {}
            if (order_id >= id) id = order_id + 1;
          }
          while (getline(in, line) && line != "[END]") {}
        }
      }
    }
  }
  return ++id;
}

bool mealAlreadySelected(const Order &o, MealType meal) {
  for (int i = 0; i < o.item_count; i++)
    if (o.items[i].type == meal)
      return true;
  return false;
}

bool addItem(Order &o, const OrderItem &item) {
  if (o.item_count >= 20) {
    cout << "Order is full. Cannot add more items.\n";
    return false;
  }
  o.items[o.item_count++] = item;
  return true;
}

void addPizza(Order &o, RestaurantData &d) {
  if (d.pizza_stock <= 0) {
    cout << "Pizza unavailable.\n";
    return;
  }
  int q = readInt("Pizza quantity: ", 1, d.pizza_stock);

  OrderItem item;
  item.type = MealType::Pizza;
  item.name = "Pizza";
  item.details = "Different Sizes";
  item.quantity = q;

  double total = 0.00;
  double cheapest = 0.00;

  for (int i = 0; i < q; i++) {
    cout << "\nPizza " << i + 1 << "\n";
    cout << "1. Large $" << d.pizza_l << "\n";
    cout << "2. Medium $" << d.pizza_m << "\n";
    cout << "3. Small $" << d.pizza_s << "\n";

    int size = readInt("Size: ", 1, 3);

    double price = size == 1 ? d.pizza_l : size == 2 ? d.pizza_m : d.pizza_s;

    total += price;

    if (i == 0 || price < cheapest)
      cheapest = price;
  }

  int freePizza = q / 3;

  if (freePizza > 0) {
    total -= cheapest * freePizza;
    cout << "Buy 2 Get 1 Free applied!\n";
  }

  if (q >= 4) {
    total *= 0.90;
    cout << "10% pizza discount applied!\n";
  }

  item.total_price = total;
  item.unit_price = total / q;

  if (addItem(o, item))
    d.pizza_stock -= q;

  cout << "Pizza added successfully!\n";
}

void addBurger(Order &o, RestaurantData &d) {
  if (d.burger_stock <= 0) {
    cout << "Burger unavailable.\n";
    return;
  }
  int q = readInt("Burger quantity: ", 1, d.burger_stock);

  cout << "1. Beef $" << d.burger_beef << "\n";
  cout << "2. Chicken $" << d.burger_chicken << "\n";

  int type = readInt("Type: ", 1, 2);

  OrderItem item;
  item.type = MealType::Burger;
  item.name = "Burger";
  item.details = type == 1 ? "Beef" : "Chicken";
  item.quantity = q;
  item.unit_price = type == 1 ? d.burger_beef : d.burger_chicken;
  item.total_price = item.unit_price * q;

  if (q == 2) {
    item.total_price -= type == 1 ? 30.00 : 40.00;
    cout << "Burger offer applied!\n";
  }

  if (addItem(o, item))
    d.burger_stock -= q;

  cout << "Burger added successfully!\n";
}

void addPotatoes(Order &o, RestaurantData &d) {
  if (d.potatoes_stock <= 0) {
    cout << "Potatoes unavailable.\n";
    return;
  }
  int q = readInt("Potatoes quantity: ", 1, d.potatoes_stock);

  OrderItem item;
  item.type = MealType::Potatoes;
  item.name = "Potatoes";
  item.details = "Regular";
  item.quantity = q;
  item.unit_price = d.potatoes;

  int freePotatoes = q / 3;
  int paid = q - freePotatoes;

  if (freePotatoes > 0)
    cout << "Buy 2 Get 1 Free applied!\n";

  item.total_price = paid * d.potatoes;

  if (q >= 4) {
    item.total_price *= 0.95;
    cout << "5% potatoes discount applied!\n";
  }

  if (addItem(o, item))
    d.potatoes_stock -= q;

  cout << "Potatoes added successfully!\n";
}

void addChicken(Order &o, RestaurantData &d) {
  if (d.chicken_stock <= 0) {
    cout << "Chicken unavailable.\n";
    return;
  }
  int q = readInt("Chicken quantity: ", 1, d.chicken_stock);

  OrderItem item;
  item.type = MealType::Chicken;
  item.name = "Chicken";
  item.details = "Different Portions";
  item.quantity = q;

  double total = 0.00;
  int fullChickenCount = 0;

  for (int i = 0; i < q; i++) {
    cout << "\nChicken " << i + 1 << "\n";
    cout << "1. Full $" << d.chicken << "\n";
    cout << "2. Half $" << d.chicken / 2 << "\n";
    cout << "3. Quarter $" << d.chicken / 4 << "\n";

    int type = readInt("Choose: ", 1, 3);

    if (type == 1) {
      total += d.chicken;
      fullChickenCount++;
    } else if (type == 2)
      total += d.chicken / 2;
    else
      total += d.chicken / 4;
  }

  if (q >= 2) {
    total *= 0.95;
    cout << "5% chicken discount applied!\n";
  }

  item.total_price = total;
  item.unit_price = total / q;

  if (!addItem(o, item))
    return;

  d.chicken_stock -= q;

  if (fullChickenCount > 0) {
    if (d.potatoes_stock >= fullChickenCount) {
      OrderItem freePotatoes;
      freePotatoes.type = MealType::Potatoes;
      freePotatoes.name = "Potatoes";
      freePotatoes.details = "Free with Full Chicken";
      freePotatoes.quantity = fullChickenCount;
      freePotatoes.unit_price = 0.00;
      freePotatoes.total_price = 0.00;

      if (addItem(o, freePotatoes)) {
        d.potatoes_stock -= fullChickenCount;
        cout << fullChickenCount << " free potatoes included!\n";
      }
    } else {
      cout << "Free potatoes unavailable due to stock.\n";
    }
  }

  cout << "Chicken added successfully!\n";
}

void chooseMeals(Order &o, RestaurantData &d) {
  int mealCount =
      readInt("\nHow many different meals do you want? (1 - 4): ", 1, 4);

  for (int i = 0; i < mealCount; i++) {
    cout << "\nSelect meal " << i + 1 << ":\n";
    cout << "1. Pizza\n";
    cout << "2. Burger\n";
    cout << "3. Potatoes\n";
    cout << "4. Chicken\n";

    int choice = readInt("Choose: ", 1, 4);

    MealType selectedMeal = static_cast<MealType>(choice);

    if (mealAlreadySelected(o, selectedMeal)) {
      cout << "You already selected this meal. "
           << "Choose another one.\n";
      i--;
      continue;
    }

    if (choice == 1)
      addPizza(o, d);
    else if (choice == 2)
      addBurger(o, d);
    else if (choice == 3)
      addPotatoes(o, d);
    else
      addChicken(o, d);
  }
}

void calculate(Order &o, const RestaurantData &d) {
  o.subtotal = 0.00;

  bool burger = false;
  bool potatoes = false;

  for (int i = 0; i < o.item_count; i++) {
    o.subtotal += o.items[i].total_price;

    if (o.items[i].type == MealType::Burger)
      burger = true;

    if (o.items[i].type == MealType::Potatoes)
      potatoes = true;
  }

  o.discount = o.customer.discount_card ? o.subtotal * 0.05 : 0.00;

  if (burger && potatoes)
    o.discount += (o.subtotal - o.discount) * 0.10;

  o.delivery_fee =
      o.customer.order_type == OrderType::Delivery ? d.delivery_fee : 0.00;

  o.tax = (o.subtotal - o.discount + o.delivery_fee) * d.tax_rate;

  o.final_total = o.subtotal - o.discount + o.delivery_fee + o.tax;
}

void saveOrderToFile(const Order &o) {
  ofstream out(ORDERS_FILE, ios::app);
  if (!out) {
    cerr << "Error: Could not open orders file for writing. Order #" << o.order_id << " not saved." << endl;
    return;
  }
  out << "[ORDER]\n";
  out << o.order_id << "\n";
  out << o.customer.name << "\n";
  out << o.customer.phone << "\n";
  out << o.customer.table_number << "\n";
  out << o.customer.discount_card << "\n";
  out << orderTypeToInt(o.customer.order_type) << "\n";
  out << o.item_count << "\n";
  for (int i = 0; i < o.item_count; i++) {
    out << mealTypeToInt(o.items[i].type) << "|"
        << o.items[i].name << "|"
        << o.items[i].details << "|"
        << o.items[i].quantity << "|"
        << o.items[i].unit_price << "|"
        << o.items[i].total_price << "\n";
  }
  out << o.subtotal << "\n";
  out << o.discount << "\n";
  out << o.tax << "\n";
  out << o.delivery_fee << "\n";
  out << o.final_total << "\n";
  out << paymentMethodToInt(o.bill.payment) << "\n";
  out << o.bill.paid_amount << "\n";
  out << "[END]\n";
}

void loadAllOrders(vector<Order> &orders) {
  orders.clear();
  ifstream in(ORDERS_FILE);
  if (!in) {
    cerr << "Warning: Could not open orders file for reading. Starting with empty order list." << endl;
    return;
  }
  string line;
  vector<int> loadedIds; // Track loaded IDs to detect duplicates
  while (getline(in, line)) {
    if (line != "[ORDER]") continue;
    Order o;
    if (!getline(in, line)) break; 
    try {
      o.order_id = stoi(line);
    } catch (...) {
      cerr << "Warning: Invalid order ID format in file. Skipping order." << endl;
      while (getline(in, line) && line != "[END]") {}
      continue;
    }
    if (!getline(in, line)) break; o.customer.name = line;
    if (!getline(in, line)) break; o.customer.phone = line;
    if (!getline(in, line)) break; 
      try { o.customer.table_number = stoi(line); } catch (...) { o.customer.table_number = 0; }
    if (!getline(in, line)) break; o.customer.discount_card = (line == "1");
    if (!getline(in, line)) break; 
      try { o.customer.order_type = intToOrderType(stoi(line)); } catch (...) { o.customer.order_type = OrderType::Takeaway; }
    if (!getline(in, line)) break; 
      try { o.item_count = stoi(line); } catch (...) { o.item_count = 0; }
    // Ensure item_count doesn't exceed array bounds
    if (o.item_count > 20) o.item_count = 20;
    if (o.item_count < 0) o.item_count = 0;
    for (int i = 0; i < o.item_count && i < 20; i++) {
      if (!getline(in, line)) break;
      stringstream ss(line);
      string token;
      vector<string> tokens;
      while (getline(ss, token, '|')) tokens.push_back(token);
      if (tokens.size() != 6) continue;
      try { o.items[i].type = intToMealType(stoi(tokens[0])); } catch (...) { o.items[i].type = MealType::Pizza; }
      o.items[i].name = tokens[1];
      o.items[i].details = tokens[2];
      try { o.items[i].quantity = stoi(tokens[3]); } catch (...) { o.items[i].quantity = 0; }
      try { o.items[i].unit_price = stod(tokens[4]); } catch (...) { o.items[i].unit_price = 0.0; }
      try { o.items[i].total_price = stod(tokens[5]); } catch (...) { o.items[i].total_price = 0.0; }
    }
    if (!getline(in, line)) break; 
      try { o.subtotal = stod(line); } catch (...) { o.subtotal = 0.0; }
    if (!getline(in, line)) break; 
      try { o.discount = stod(line); } catch (...) { o.discount = 0.0; }
    if (!getline(in, line)) break; 
      try { o.tax = stod(line); } catch (...) { o.tax = 0.0; }
    if (!getline(in, line)) break; 
      try { o.delivery_fee = stod(line); } catch (...) { o.delivery_fee = 0.0; }
    if (!getline(in, line)) break; 
      try { o.final_total = stod(line); } catch (...) { o.final_total = 0.0; }
    if (!getline(in, line)) break; 
      try { o.bill.payment = intToPaymentMethod(stoi(line)); } catch (...) { o.bill.payment = PaymentMethod::Cash; }
    if (!getline(in, line)) break; 
      try { o.bill.paid_amount = stod(line); } catch (...) { o.bill.paid_amount = 0.0; }
    if (!getline(in, line)) break; // skip [END]
    
    // Check for duplicate order ID
    bool isDuplicate = false;
    for (int existingId : loadedIds) {
      if (existingId == o.order_id) {
        isDuplicate = true;
        break;
      }
    }
    
    if (isDuplicate) {
      cerr << "Warning: Duplicate order ID #" << o.order_id << " found in file. Skipping duplicate." << endl;
    } else {
      orders.push_back(o);
      loadedIds.push_back(o.order_id);
    }
  }
}

Order* findOrderById(vector<Order> &orders, int id) {
  for (auto &o : orders) {
    if (o.order_id == id) return &o;
  }
  return nullptr;
}

vector<Order> findOrdersByCustomer(vector<Order> &orders, const string &name) {
  vector<Order> result;
  for (auto &o : orders) {
    if (o.customer.name == name) result.push_back(o);
  }
  return result;
}

void showOrder(Order &o, const RestaurantData &d) {
  calculate(o, d);

  cout << "\n========== ORDER ==========\n";
  cout << fixed << setprecision(2);

  cout << "ID: #" << o.order_id << "\n";
  cout << "Customer: " << o.customer.name << "\n";

  for (int i = 0; i < o.item_count; i++) {
    cout << i + 1 << ". " << o.items[i].name << " (" << o.items[i].details
         << ")"
         << " x" << o.items[i].quantity << " = $" << o.items[i].total_price
         << "\n";
  }

  cout << "Subtotal: $" << o.subtotal << "\n";
  cout << "Discount: -$" << o.discount << "\n";
  cout << "Delivery: $" << o.delivery_fee << "\n";
  cout << "Tax: $" << o.tax << "\n";
  cout << "FINAL TOTAL: $" << o.final_total << "\n";

  cout << "===========================\n";
}

PaymentMethod paymentMethod() {
  cout << "\n1. Cash\n";
  cout << "2. Credit Card\n";
  cout << "3. Wallet\n";

  return static_cast<PaymentMethod>(readInt("Payment: ", 1, 3));
}

void printPayment(PaymentMethod p) {
  cout << "Payment: ";

  switch (p) {
  case PaymentMethod::Cash:
    cout << "Cash\n";
    break;

  case PaymentMethod::CreditCard:
    cout << "Credit Card\n";
    break;

  case PaymentMethod::Wallet:
    cout << "Wallet\n";
    break;
  }
}

void payment(Order &o, RestaurantData &d) {
  o.bill.payment = paymentMethod();

  if (o.bill.payment == PaymentMethod::CreditCard ||
      o.bill.payment == PaymentMethod::Wallet)
    o.bill.paid_amount = o.final_total;
  else {
    do {
      o.bill.paid_amount = readPositive("Paid amount: ");

      if (o.bill.paid_amount < o.final_total)
        cout << "Not enough money!\n";

    } while (o.bill.paid_amount < o.final_total);
  }

  d.total_orders++;
  d.total_sales += o.final_total;

  if (o.bill.payment == PaymentMethod::Cash)
    d.cash_orders++;
  else if (o.bill.payment == PaymentMethod::CreditCard)
    d.card_orders++;
  else
    d.wallet_orders++;
}

void invoice(const Order &o) {
  cout << "\n========================================\n";
  cout << "             RESTAURANT BILL\n";
  cout << "========================================\n";
  cout << fixed << setprecision(2);

  cout << "Order ID: #" << o.order_id << "\n";
  cout << "Customer: " << o.customer.name << "\n";
  cout << "Phone: " << o.customer.phone << "\n";

  cout << "Order Type: ";

  if (o.customer.order_type == OrderType::DineIn) {
    cout << "Dine-in\n";
    cout << "Table: " << o.customer.table_number << "\n";
  } else if (o.customer.order_type == OrderType::Takeaway)
    cout << "Takeaway\n";
  else
    cout << "Delivery\n";

  cout << "\n---------- ITEMS ----------\n";

  for (int i = 0; i < o.item_count; i++) {
    cout << o.items[i].name << " (" << o.items[i].details << ")"
         << " x" << o.items[i].quantity << " = $" << o.items[i].total_price
         << "\n";
  }

  cout << "\nSubtotal: $" << o.subtotal << "\n";
  cout << "Discount: -$" << o.discount << "\n";
  cout << "Delivery: $" << o.delivery_fee << "\n";
  cout << "Tax: $" << o.tax << "\n";
  cout << "FINAL TOTAL: $" << o.final_total << "\n";

  printPayment(o.bill.payment);

  cout << "Paid: $" << o.bill.paid_amount << "\n";
  cout << "Change: $" << o.bill.paid_amount - o.final_total << "\n";

  cout << "========================================\n";
  cout << "              THANK YOU!\n";
  cout << "========================================\n";
}

void rating() {
  int r = readInt("\nRate us (1 - 5): ", 1, 5);

  cout << "Your rating: ";

  for (int i = 0; i < r; i++)
    cout << "* ";

  cout << "\nThank you!\n";
}

void salesReport(const RestaurantData &d) {
  cout << "\n========== SALES REPORT ==========\n";
  cout << fixed << setprecision(2);

  cout << "Total Orders: " << d.total_orders << "\n";
  cout << "Total Sales: $" << d.total_sales << "\n";
  cout << "Cash: " << d.cash_orders << "\n";
  cout << "Credit Card: " << d.card_orders << "\n";
  cout << "Wallet: " << d.wallet_orders << "\n";

  cout << "Average Order: $"
       << (d.total_orders > 0 ? d.total_sales / d.total_orders : 0.00) << "\n";

  cout << "==================================\n";
}

void adminMenu(RestaurantData &d) {
  int choice;

  do {
    cout << "\n========== ADMIN ==========\n";
    cout << "1. View Menu\n";
    cout << "2. Change Pizza Prices\n";
    cout << "3. Change Burger Prices\n";
    cout << "4. Change Potatoes Price\n";
    cout << "5. Change Chicken Price\n";
    cout << "6. View Inventory\n";
    cout << "7. Sales Report\n";
    cout << "8. Change Delivery Fee\n";
    cout << "9. Search Order by ID\n";
    cout << "10. Search Orders by Customer\n";
    cout << "11. Reprint Invoice\n";
    cout << "12. Exit\n";

    choice = readInt("Choose: ", 1, 12);

    if (choice == 1) {
      showMenu(d);
    } else if (choice == 2) {
      d.pizza_l = readPositive("Large: ");
      d.pizza_m = readPositive("Medium: ");
      d.pizza_s = readPositive("Small: ");
    } else if (choice == 3) {
      d.burger_beef = readPositive("Beef: ");
      d.burger_chicken = readPositive("Chicken: ");
    } else if (choice == 4) {
      d.potatoes = readPositive("Potatoes: ");
    } else if (choice == 5) {
      d.chicken = readPositive("Chicken: ");
    } else if (choice == 6) {
      cout << "\n========== INVENTORY ==========\n";
      cout << "Pizza: " << d.pizza_stock << "\n";
      cout << "Burger: " << d.burger_stock << "\n";
      cout << "Potatoes: " << d.potatoes_stock << "\n";
      cout << "Chicken: " << d.chicken_stock << "\n";
    } else if (choice == 7) {
      salesReport(d);
    } else if (choice == 8) {
      d.delivery_fee = readPositive("Delivery Fee: ");
    } else if (choice == 9) {
      int id = readInt("Enter order ID: ", 1000, 9999);
      Order *o = findOrderById(d.completed_orders, id);
      if (o) {
        showOrder(*o, d);
      } else {
        cout << "Order not found.\n";
      }
    } else if (choice == 10) {
      string name = readName();
      vector<Order> results = findOrdersByCustomer(d.completed_orders, name);
      if (results.empty()) {
        cout << "No orders found for " << name << "\n";
      } else {
        for (auto &order : results) {
          showOrder(order, d);
        }
      }
    } else if (choice == 11) {
      int id = readInt("Enter order ID to reprint: ", 1000, 9999);
      Order *o = findOrderById(d.completed_orders, id);
      if (o) {
        invoice(*o);
      } else {
        cout << "Order not found.\n";
      }
    }

  } while (choice != 12);
}

void restoreStock(const Order &o, RestaurantData &d) {
  for (int i = 0; i < o.item_count; i++) {
    if (o.items[i].type == MealType::Pizza)
      d.pizza_stock += o.items[i].quantity;
    else if (o.items[i].type == MealType::Burger)
      d.burger_stock += o.items[i].quantity;
    else if (o.items[i].type == MealType::Potatoes)
      d.potatoes_stock += o.items[i].quantity;
    else if (o.items[i].type == MealType::Chicken)
      d.chicken_stock += o.items[i].quantity;
  }
}

void createOrder(RestaurantData &d) {
  Order o;

  o.order_id = generateID();

  customerInfo(o.customer);
  chooseMeals(o, d);

  if (o.item_count == 0) {
    cout << "No valid meals were added.\n";
    return;
  }

  showOrder(o, d);

  if (!yesNo("Confirm order? (1 Yes / 0 No): ")) {
    restoreStock(o, d);
    cout << "Order cancelled.\n";
    return;
  }

  payment(o, d);
  d.completed_orders.push_back(o);
  saveOrderToFile(o);
  invoice(o);
  rating();
}

void mainMenu(RestaurantData &d) {
  int choice;

  do {
    cout << "\n========================================\n";
    cout << "       RESTAURANT MANAGEMENT SYSTEM\n";
    cout << "========================================\n";

    cout << "1. Customer Order\n";
    cout << "2. View Menu\n";
    cout << "3. Admin Panel\n";
    cout << "4. Sales Report\n";
    cout << "5. Exit\n";

    choice = readInt("Choose: ", 1, 5);

    if (choice == 1)
      createOrder(d);
    else if (choice == 2)
      showMenu(d);
    else if (choice == 3)
      adminMenu(d);
    else if (choice == 4)
      salesReport(d);

  } while (choice != 5);

  cout << "\nThank you for using our system!\n";
}

int main() {
  RestaurantData data;
  loadAllOrders(data.completed_orders);
  mainMenu(data);
  return 0;
}