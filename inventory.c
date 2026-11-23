
#include "game.h"

// ============================================================================
// SHOP, GACHA, INVENTORY & SELLING SYSTEM
// All changes are contained in inventory.c.
// ============================================================================

#define GACHA_BASE_COST 100
#define GACHA_ROLL_STEP 25
#define LEVEL_PRICE_STEP 10
#define SELL_PERCENT 40

static struct Item shopCatalog[] = {
    {101, "Small Health Potion", ITEM_POTION, 40,  25,  1},
    {102, "Mega Health Potion",  ITEM_POTION, 100, 60,  1},
    {201, "Iron Sword",     ITEM_WEAPON, 8,   120, 1},
    {202, "Steel Claymore",      ITEM_WEAPON, 16,  280, 1},
    {301, "Leather Tunic",       ITEM_ARMOR,  5,   1000, 1},
    {302, "Knight Chainmail",    ITEM_ARMOR,  12,  250, 1}
};

#define SHOP_ITEM_COUNT \
    ((int)(sizeof(shopCatalog) / sizeof(shopCatalog[0])))

// Gacha reward pool: item values range from 5 G to 1000 G.
static struct Item gachaPool[] = {
    {401, "Stupid Stick", ITEM_WEAPON, 1, 5, 1},
    {402, "Cracked Pebble", ITEM_WEAPON, 2, 15, 1},
    {403, "Bent Dagger", ITEM_WEAPON, 4, 35, 1},
    {404, "Wooden Sword", ITEM_WEAPON, 6, 60, 1},
    {405, "Lucky Charm", ITEM_ARMOR, 2, 90, 1},
    {406, "Copper Blade", ITEM_WEAPON, 9, 130, 1},
    {407, "Traveler Vest", ITEM_ARMOR, 5, 180, 1},
    {408, "Knight's Edge", ITEM_WEAPON, 15, 250, 1},
    {409, "Guardian Plate", ITEM_ARMOR, 10, 350, 1},
    {410, "Runic Greatsword", ITEM_WEAPON, 22, 500, 1},
    {411, "Phoenix Mail", ITEM_ARMOR, 16, 700, 1},
    {412, "Dragon King's Fang", ITEM_WEAPON, 30, 1000, 1}
};

#define GACHA_ITEM_COUNT \
    ((int)(sizeof(gachaPool) / sizeof(gachaPool[0])))

// ============================================================================
// INVENTORY HELPERS
// ============================================================================

static void removeInventoryItem(struct Player *p, int index) {
    if (index < 0 || index >= p->item_count)
        return;

    for (int i = index; i < p->item_count - 1; i++) {
        p->inventory[i] = p->inventory[i + 1];
    }

    p->item_count--;
}

static int findItemIndex(const struct Player *p, int item_id) {
    for (int i = 0; i < p->item_count; i++) {
        if (p->inventory[i].item_id == item_id)
            return i;
    }

    return -1;
}

static int addItemToInventory(struct Player *p, struct Item item) {
    int existing = findItemIndex(p, item.item_id);

    // Potions can stack.
    // Weapons and armor cannot stack.
    if (existing >= 0 && item.type == ITEM_POTION) {
        p->inventory[existing].quantity += item.quantity;
        return 1;
    }

    if (existing >= 0 || p->item_count >= MAX_INVENTORY)
        return 0;

    item.quantity = 1;
    p->inventory[p->item_count++] = item;

    return 1;
}

// ============================================================================
// PRICE SCALING
// ============================================================================

// Shop prices increase by 10% per level above level 1.
static int scaledPrice(const struct Player *p, int base_price) {
    int level = p->level > 1 ? p->level : 1;

    return base_price *
           (100 + (level - 1) * LEVEL_PRICE_STEP) / 100;
}

// Selling price is always lower than buying price.
static int sellPrice(const struct Player *p, const struct Item *item) {
    int buy_value = scaledPrice(p, item->price);
    int value = buy_value * SELL_PERCENT / 100;

    return value < 1 ? 1 : value;
}

static const char *itemTypeName(int type) {
    if (type == ITEM_POTION)
        return "Potion";

    if (type == ITEM_WEAPON)
        return "Weapon";

    return "Armor";
}

// ============================================================================
// BUYING SYSTEM
// ============================================================================

static void buyFromShop(struct Player *p) {
    while (1) {
        clearScreen();

        printf("=============== GENERAL STORE ===============\n");
        printf("Gold: %d G | Level: %d | Inventory: %d/%d\n",
               p->gold, p->level, p->item_count, MAX_INVENTORY);

        printf("----------------------------------------------\n");
        printf(" No | Item                 | Type   | Effect | Price\n");

        for (int i = 0; i < SHOP_ITEM_COUNT; i++) {
            printf("%3d | %-20s | %-6s | +%-5d | %d G\n",
                   i + 1,
                   shopCatalog[i].name,
                   itemTypeName(shopCatalog[i].type),
                   shopCatalog[i].stat_value,
                   scaledPrice(p, shopCatalog[i].price));
        }

        printf("----------------------------------------------\n");
        printf("0. Back\n");
        printf("Choose item: ");

        int choice = getValidIntInput(0, SHOP_ITEM_COUNT);

        if (choice == 0)
            return;

        struct Item item = shopCatalog[choice - 1];
        int price = scaledPrice(p, item.price);

        if (p->gold < price) {
            printf("\nNot enough gold! You need %d G more.\n",
                   price - p->gold);
        }
        else if (!addItemToInventory(p, item)) {
            printf("\nInventory full or you already own this equipment.\n");
        }
        else {
            p->gold -= price;

            printf("\nPurchased %s for %d G.\n",
                   item.name, price);
        }

        pauseScreen();
    }
}

// ============================================================================
// SELLING SYSTEM
// ============================================================================

static void sellFromShop(struct Player *p) {
    while (1) {
        clearScreen();

        printf("================ SELL ITEMS ================\n");
        printf("Gold: %d G | Sell value: %d%% of shop price\n",
               p->gold, SELL_PERCENT);

        if (p->item_count == 0) {
            printf("Your inventory is empty.\n");
            pauseScreen();
            return;
        }

        printf("---------------------------------------------\n");
        printf(" No | Item                 | Qty | Sell price\n");

        for (int i = 0; i < p->item_count; i++) {
            struct Item *it = &p->inventory[i];

            printf("%3d | %-20s | %3d | %d G each%s\n",
                   i + 1,
                   it->name,
                   it->quantity,
                   sellPrice(p, it),
                   ((it->type == ITEM_WEAPON &&
                     p->equipped_weapon_id == it->item_id) ||
                    (it->type == ITEM_ARMOR &&
                     p->equipped_armor_id == it->item_id))
                       ? " [EQUIPPED]" : "");
        }

        printf("---------------------------------------------\n");
        printf("0. Back\n");
        printf("Choose item to sell: ");

        int choice = getValidIntInput(0, p->item_count);

        if (choice == 0)
            return;

        int idx = choice - 1;
        struct Item *it = &p->inventory[idx];

        int price = sellPrice(p, it);

        // Automatically unequip equipment before selling.
        if (it->type == ITEM_WEAPON &&
            p->equipped_weapon_id == it->item_id) {
            p->equipped_weapon_id = 0;
            p->bonus_atk = 0;
        }

        if (it->type == ITEM_ARMOR &&
            p->equipped_armor_id == it->item_id) {
            p->equipped_armor_id = 0;
            p->bonus_def = 0;
        }

        // Sell one unit at a time.
        p->gold += price;
        it->quantity--;

        if (it->quantity <= 0)
            removeInventoryItem(p, idx);

        printf("\nSold 1 item for %d G.\n", price);

        pauseScreen();
    }
}

// ============================================================================
// GACHA SYSTEM
// ============================================================================

static void rollGacha(struct Player *p, int *roll_count) {

    int cost = GACHA_BASE_COST
             + (*roll_count * GACHA_ROLL_STEP)
             + ((p->level > 1 ? p->level - 1 : 0)
                * LEVEL_PRICE_STEP);

    clearScreen();

    printf("================= ITEM GACHA =================\n");
    printf("Gold: %d G | Roll number: %d\n",
           p->gold, *roll_count + 1);

    printf("Next roll cost: %d G\n", cost);
    printf("Possible item value: 5 - 1000 G\n");
    printf("----------------------------------------------\n");

    if (p->gold < cost) {
        printf("Not enough gold! You need %d G more.\n",
               cost - p->gold);

        pauseScreen();
        return;
    }

    if (p->item_count >= MAX_INVENTORY) {
        printf("Inventory full. Make room before rolling.\n");

        pauseScreen();
        return;
    }

    // Deduct gold.
    p->gold -= cost;
    (*roll_count)++;

    // Random reward.
    int index = rand() % GACHA_ITEM_COUNT;
    struct Item reward = gachaPool[index];

    if (!addItemToInventory(p, reward)) {

        // Refund if the reward cannot be added.
        p->gold += cost;
        (*roll_count)--;

        printf("You already own this equipment. Roll refunded.\n");
    }
    else {
        printf("\n*** GACHA RESULT ***\n");

        printf("You got: %s (%s)\n",
               reward.name,
               itemTypeName(reward.type));

        printf("Effect: +%d\n", reward.stat_value);
        printf("Item value: %d G\n", reward.price);
        printf("Gold remaining: %d G\n", p->gold);
    }

    pauseScreen();
}

// ============================================================================
// MAIN SHOP MENU
// ============================================================================

void openShop(struct Player *p) {

    int roll_count = 0;

    while (1) {
        clearScreen();

        printf("=============== ADVENTURER HUB ===============\n");
        printf("Gold: %d G | Level: %d | Inventory: %d/%d\n",
               p->gold, p->level, p->item_count, MAX_INVENTORY);

        printf("----------------------------------------------\n");
        printf("1. Buy items (level-scaled prices)\n");
        printf("2. Sell items (40%% of buy price)\n");
        printf("3. Gacha roll (starts at 100 G; price scales)\n");
        printf("0. Leave\n");

        printf("Choose: ");

        int choice = getValidIntInput(0, 3);

        if (choice == 0)
            return;

        if (choice == 1)
            buyFromShop(p);

        else if (choice == 2)
            sellFromShop(p);

        else
            rollGacha(p, &roll_count);
    }
}

// ============================================================================
// INVENTORY MENU
// ============================================================================

void openInventory(struct Player *p) {

    while (1) {
        clearScreen();

        printf("================= INVENTORY =================\n");
        printf("HP: %d/%d | ATK bonus: +%d | DEF bonus: +%d\n",
               p->hp, p->max_hp, p->bonus_atk, p->bonus_def);

        printf("Gold: %d G | Slots: %d/%d\n",
               p->gold, p->item_count, MAX_INVENTORY);

        printf("---------------------------------------------\n");

        if (p->item_count == 0) {
            printf("Inventory is empty.\n");
            pauseScreen();
            return;
        }

        printf(" No | Item                 | Type   | Effect | Qty | Status\n");

        for (int i = 0; i < p->item_count; i++) {
            struct Item *it = &p->inventory[i];

            const char *status = "";

            if (it->type == ITEM_WEAPON &&
                p->equipped_weapon_id == it->item_id) {
                status = "[EQUIPPED]";
            }
            else if (it->type == ITEM_ARMOR &&
                     p->equipped_armor_id == it->item_id) {
                status = "[EQUIPPED]";
            }

            printf("%3d | %-20s | %-6s | +%-5d | %3d | %s\n",
                   i + 1,
                   it->name,
                   itemTypeName(it->type),
                   it->stat_value,
                   it->quantity,
                   status);
        }

        printf("---------------------------------------------\n");
        printf("0. Close inventory\n");
        printf("Choose item to use/equip: ");

        int choice = getValidIntInput(0, p->item_count);

        if (choice == 0)
            return;

        int idx = choice - 1;
        struct Item *chosen = &p->inventory[idx];

        if (chosen->type == ITEM_POTION) {

            if (p->hp >= p->max_hp) {
                printf("\nHP is already full (%d/%d).\n",
                       p->hp, p->max_hp);
            }
            else {
                int old_hp = p->hp;

                p->hp += chosen->stat_value;

                if (p->hp > p->max_hp)
                    p->hp = p->max_hp;

                printf("\nUsed %s. HP restored: %d (%d -> %d).\n",
                       chosen->name,
                       p->hp - old_hp,
                       old_hp,
                       p->hp);

                chosen->quantity--;

                if (chosen->quantity <= 0)
                    removeInventoryItem(p, idx);
            }
        }

        else if (chosen->type == ITEM_WEAPON) {

            p->equipped_weapon_id = chosen->item_id;
            p->bonus_atk = chosen->stat_value;

            printf("\nEquipped %s. ATK bonus: +%d.\n",
                   chosen->name, p->bonus_atk);
        }

        else if (chosen->type == ITEM_ARMOR) {

            p->equipped_armor_id = chosen->item_id;
            p->bonus_def = chosen->stat_value;

            printf("\nEquipped %s. DEF bonus: +%d.\n",
                   chosen->name, p->bonus_def);
        }

        pauseScreen();
    }
}

// ============================================================================
// COMBAT POTION USAGE
// ============================================================================

int usePotionInCombat(struct Player *p) {

    if (p->hp >= p->max_hp) {
        printf(">> HP is already full!\n");
        return 0;
    }

    int potion_idx = -1;

    for (int i = 0; i < p->item_count; i++) {

        if (p->inventory[i].type == ITEM_POTION &&
            p->inventory[i].quantity > 0) {

            potion_idx = i;
            break;
        }
    }

    if (potion_idx == -1) {
        printf(">> You do not have any Potions!\n");
        return 0;
    }

    struct Item *pot = &p->inventory[potion_idx];

    int old_hp = p->hp;

    p->hp += pot->stat_value;

    if (p->hp > p->max_hp)
        p->hp = p->max_hp;

    printf(">> %s used %s! HP restored +%d (%d/%d).\n",
        p->name,
        pot->name,
        p->hp - old_hp,
        p->hp,
        p->max_hp);

    pot->quantity--;

    if (pot->quantity <= 0)
        removeInventoryItem(p, potion_idx);

    return 1;
}
