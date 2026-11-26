#include "game.h"

#define GACHA_BASE_COST 100
#define GACHA_ROLL_STEP 25
#define LEVEL_PRICE_STEP 10
#define SELL_PERCENT 40
#define SHOP_ITEM_COUNT 6
#define GACHA_ITEM_COUNT 12

// Daftar barang di toko
struct Item shopCatalog[SHOP_ITEM_COUNT] = {
    {101, "Small Health Potion", ITEM_POTION, 40,  25,   1},
    {102, "Mega Health Potion",  ITEM_POTION, 100, 60,   1},
    {201, "Iron Sword",          ITEM_WEAPON, 8,   120,  1},
    {202, "Steel Claymore",      ITEM_WEAPON, 16,  280,  1},
    {301, "Leather Tunic",       ITEM_ARMOR,  5,   1000, 1},
    {302, "Knight Chainmail",    ITEM_ARMOR,  12,  250,  1}
};

// Daftar hadiah gacha (nilai 5 G - 1000 G)
struct Item gachaPool[GACHA_ITEM_COUNT] = {
    {401, "Stupid Stick",       ITEM_WEAPON, 1,  5,    1},
    {402, "Cracked Pebble",     ITEM_WEAPON, 2,  15,   1},
    {403, "Bent Dagger",        ITEM_WEAPON, 4,  35,   1},
    {404, "Wooden Sword",       ITEM_WEAPON, 6,  60,   1},
    {405, "Lucky Charm",        ITEM_ARMOR,  2,  90,   1},
    {406, "Copper Blade",       ITEM_WEAPON, 9,  130,  1},
    {407, "Traveler Vest",      ITEM_ARMOR,  5,  180,  1},
    {408, "Knight's Edge",      ITEM_WEAPON, 15, 250,  1},
    {409, "Guardian Plate",     ITEM_ARMOR,  10, 350,  1},
    {410, "Runic Greatsword",   ITEM_WEAPON, 22, 500,  1},
    {411, "Phoenix Mail",       ITEM_ARMOR,  16, 700,  1},
    {412, "Dragon King's Fang", ITEM_WEAPON, 30, 1000, 1}
};

// Hapus item dari tas berdasarkan indeks lalu geser sisanya
void removeInventoryItem(struct Player *p, int index) {
    if (index < 0 || index >= p->item_count) {
        return;
    }

    for (int i = index; i < p->item_count - 1; i++) {
        p->inventory[i] = p->inventory[i + 1];
    }

    p->item_count--;
}

// Cari indeks item di tas berdasarkan ID, return -1 jika tidak ada
int findItemIndex(struct Player *p, int item_id) {
    for (int i = 0; i < p->item_count; i++) {
        if (p->inventory[i].item_id == item_id) {
            return i;
        }
    }
    return -1;
}

// Tambah item ke tas (potion bisa ditumpuk, senjata & armor tidak)
int addItemToInventory(struct Player *p, struct Item item) {
    int indeks_ada = findItemIndex(p, item.item_id);

    if (indeks_ada >= 0 && item.type == ITEM_POTION) {
        p->inventory[indeks_ada].quantity += item.quantity;
        return 1;
    }

    if (indeks_ada >= 0 || p->item_count >= MAX_INVENTORY) {
        return 0;
    }

    item.quantity = 1;
    p->inventory[p->item_count] = item;
    p->item_count++;

    return 1;
}

// Hitung harga beli sesuai level pemain (+10% tiap naik level)
int scaledPrice(struct Player *p, int base_price) {
    int level = p->level;
    if (level < 1) {
        level = 1;
    }

    int harga = base_price * (100 + (level - 1) * LEVEL_PRICE_STEP) / 100;
    return harga;
}

// Hitung harga jual (40% dari harga beli saat ini, minimal 1 G)
int sellPrice(struct Player *p, struct Item *item) {
    int harga_beli = scaledPrice(p, item->price);
    int harga_jual = harga_beli * SELL_PERCENT / 100;

    if (harga_jual < 1) {
        harga_jual = 1;
    }
    return harga_jual;
}

// Ambil teks tipe item
void getItemTypeName(int type, char teks[]) {
    if (type == ITEM_POTION) {
        strcpy(teks, "Potion");
    } else if (type == ITEM_WEAPON) {
        strcpy(teks, "Weapon");
    } else {
        strcpy(teks, "Armor");
    }
}

// Menu beli barang di toko
void buyFromShop(struct Player *p) {
    while (1) {
        clearScreen();
        printf("=============== TOKO PERALATAN ===============\n");
        printf("Gold: %d G | Level: %d | Tas: %d/%d\n",
               p->gold, p->level, p->item_count, MAX_INVENTORY);
        printf("----------------------------------------------\n");
        printf(" No | Item                 | Tipe   | Efek   | Harga\n");

        for (int i = 0; i < SHOP_ITEM_COUNT; i++) {
            char nama_tipe[15];
            getItemTypeName(shopCatalog[i].type, nama_tipe);
            int harga = scaledPrice(p, shopCatalog[i].price);

            printf("%3d | %-20s | %-6s | +%-5d | %d G\n",
                   i + 1,
                   shopCatalog[i].name,
                   nama_tipe,
                   shopCatalog[i].stat_value,
                   harga);
        }

        printf("----------------------------------------------\n");
        printf("0. Kembali\n");
        printf("Pilih item: ");
        int pilihan = getValidIntInput(0, SHOP_ITEM_COUNT);

        if (pilihan == 0) {
            return;
        }

        struct Item item = shopCatalog[pilihan - 1];
        int harga = scaledPrice(p, item.price);

        if (p->gold < harga) {
            printf("\nGold tidak cukup! Kurang %d G lagi.\n", harga - p->gold);
        } else if (addItemToInventory(p, item) == 0) {
            printf("\nTas penuh atau kamu sudah punya equipment ini.\n");
        } else {
            p->gold -= harga;
            printf("\nBerhasil membeli %s seharga %d G.\n", item.name, harga);
        }

        pauseScreen();
    }
}

// Menu jual barang dari tas
void sellFromShop(struct Player *p) {
    while (1) {
        clearScreen();
        printf("================ JUAL BARANG ================\n");
        printf("Gold: %d G | Harga jual: %d%% dari harga toko\n",
               p->gold, SELL_PERCENT);

        if (p->item_count == 0) {
            printf("Tas kamu kosong.\n");
            pauseScreen();
            return;
        }

        printf("---------------------------------------------\n");
        printf(" No | Item                 | Jml | Harga Jual\n");

        for (int i = 0; i < p->item_count; i++) {
            struct Item *it = &p->inventory[i];
            int harga_jual = sellPrice(p, it);

            char status[15] = "";
            if (it->type == ITEM_WEAPON && p->equipped_weapon_id == it->item_id) {
                strcpy(status, " [DIPAKAI]");
            } else if (it->type == ITEM_ARMOR && p->equipped_armor_id == it->item_id) {
                strcpy(status, " [DIPAKAI]");
            }

            printf("%3d | %-20s | %3d | %d G/item%s\n",
                   i + 1,
                   it->name,
                   it->quantity,
                   harga_jual,
                   status);
        }

        printf("---------------------------------------------\n");
        printf("0. Kembali\n");
        printf("Pilih item yang ingin dijual: ");
        int pilihan = getValidIntInput(0, p->item_count);

        if (pilihan == 0) {
            return;
        }

        int idx = pilihan - 1;
        struct Item *it = &p->inventory[idx];
        int harga = sellPrice(p, it);

        // Lepas equipment otomatis jika sedang dipakai
        if (it->type == ITEM_WEAPON && p->equipped_weapon_id == it->item_id) {
            p->equipped_weapon_id = 0;
            p->bonus_atk = 0;
        }
        if (it->type == ITEM_ARMOR && p->equipped_armor_id == it->item_id) {
            p->equipped_armor_id = 0;
            p->bonus_def = 0;
        }

        // Jual 1 buah per transaksi
        p->gold += harga;
        it->quantity--;

        if (it->quantity <= 0) {
            removeInventoryItem(p, idx);
        }

        printf("\nBerhasil menjual 1 item seharga %d G.\n", harga);
        pauseScreen();
    }
}

// Sistem gacha item
void rollGacha(struct Player *p, int *roll_count) {
    int faktor_level = 0;
    if (p->level > 1) {
        faktor_level = p->level - 1;
    }

    int biaya = GACHA_BASE_COST + (*roll_count * GACHA_ROLL_STEP) + (faktor_level - LEVEL_PRICE_STEP);

    clearScreen();
    printf("================= GACHA ITEM =================\n");
    printf("Gold: %d G | Roll ke-%d\n", p->gold, *roll_count + 1);
    printf("Biaya roll ini: %d G\n", biaya);
    printf("Hadiah item senilai: 5 - 1000 G\n");
    printf("----------------------------------------------\n");

    if (p->gold < biaya) {
        printf("Gold tidak cukup! Kurang %d G lagi.\n", biaya - p->gold);
        pauseScreen();
        return;
    }

    if (p->item_count >= MAX_INVENTORY) {
        printf("Tas penuh. Kosongkan slot dulu sebelum gacha.\n");
        pauseScreen();
        return;
    }

    p->gold -= biaya;
    (*roll_count)++;

    int indeks_acak = rand() % GACHA_ITEM_COUNT;
    struct Item hadiah = gachaPool[indeks_acak];

    if (addItemToInventory(p, hadiah) == 0) {
        // Kembalikan gold jika item duplikat/gagal masuk tas
        p->gold += biaya;
        (*roll_count)--;
        printf("Kamu sudah punya equipment ini. Gold dikembalikan.\n");
    } else {
        char nama_tipe[15];
        getItemTypeName(hadiah.type, nama_tipe);

        printf("\n*** HASIL GACHA ***\n");
        printf("Kamu mendapat: %s (%s)\n", hadiah.name, nama_tipe);
        printf("Efek: +%d\n", hadiah.stat_value);
        printf("Nilai item: %d G\n", hadiah.price);
        printf("Sisa Gold: %d G\n", p->gold);
    }

    pauseScreen();
}

// Menu utama toko
void openShop(struct Player *p) {
    int roll_count = 0;

    while (1) {
        clearScreen();
        printf("=============== ADVENTURER HUB ===============\n");
        printf("Gold: %d G | Level: %d | Tas: %d/%d\n",
               p->gold, p->level, p->item_count, MAX_INVENTORY);
        printf("----------------------------------------------\n");
        printf("1. Beli item (harga menyesuaikan level)\n");
        printf("2. Jual item (40%% dari harga beli)\n");
        printf("3. Gacha item (mulai 100 G, harga naik tiap roll)\n");
        printf("0. Keluar\n");
        printf("Pilihan: ");
        int pilihan = getValidIntInput(0, 3);

        if (pilihan == 0) {
            return;
        } else if (pilihan == 1) {
            buyFromShop(p);
        } else if (pilihan == 2) {
            sellFromShop(p);
        } else {
            rollGacha(p, &roll_count);
        }
    }
}

// Menu tas / inventory
void openInventory(struct Player *p) {
    while (1) {
        clearScreen();
        printf("================= INVENTORY =================\n");
        printf("HP: %d/%d | Bonus ATK: +%d | Bonus DEF: +%d\n",
               p->hp, p->max_hp, p->bonus_atk, p->bonus_def);
        printf("Gold: %d G | Slot: %d/%d\n",
               p->gold, p->item_count, MAX_INVENTORY);
        printf("---------------------------------------------\n");

        if (p->item_count == 0) {
            printf("Tas kosong.\n");
            pauseScreen();
            return;
        }

        printf(" No | Item                 | Tipe   | Efek   | Jml | Status\n");

        for (int i = 0; i < p->item_count; i++) {
            struct Item *it = &p->inventory[i];
            char nama_tipe[15];
            getItemTypeName(it->type, nama_tipe);

            char status[15] = "";
            if (it->type == ITEM_WEAPON && p->equipped_weapon_id == it->item_id) {
                strcpy(status, "[DIPAKAI]");
            } else if (it->type == ITEM_ARMOR && p->equipped_armor_id == it->item_id) {
                strcpy(status, "[DIPAKAI]");
            }

            printf("%3d | %-20s | %-6s | +%-5d | %3d | %s\n",
                   i + 1,
                   it->name,
                   nama_tipe,
                   it->stat_value,
                   it->quantity,
                   status);
        }

        printf("---------------------------------------------\n");
        printf("0. Tutup tas\n");
        printf("Pilih item untuk dipakai/dipasang: ");
        int pilihan = getValidIntInput(0, p->item_count);

        if (pilihan == 0) {
            return;
        }

        int idx = pilihan - 1;
        struct Item *dipilih = &p->inventory[idx];

        if (dipilih->type == ITEM_POTION) {
            if (p->hp >= p->max_hp) {
                printf("\nHP sudah penuh (%d/%d).\n", p->hp, p->max_hp);
            } else {
                int hp_lama = p->hp;
                p->hp += dipilih->stat_value;

                if (p->hp > p->max_hp) {
                    p->hp = p->max_hp;
                }

                printf("\nMenggunakan %s. HP pulih +%d (%d -> %d).\n",
                       dipilih->name, p->hp - hp_lama, hp_lama, p->hp);

                dipilih->quantity--;
                if (dipilih->quantity <= 0) {
                    removeInventoryItem(p, idx);
                }
            }
        } else if (dipilih->type == ITEM_WEAPON) {
            p->equipped_weapon_id = dipilih->item_id;
            p->bonus_atk = dipilih->stat_value;
            printf("\nMemasang %s. Bonus ATK: +%d.\n", dipilih->name, p->bonus_atk);
        } else if (dipilih->type == ITEM_ARMOR) {
            p->equipped_armor_id = dipilih->item_id;
            p->bonus_def = dipilih->stat_value;
            printf("\nMemasang %s. Bonus DEF: +%d.\n", dipilih->name, p->bonus_def);
        }

        pauseScreen();
    }
}

// Pakai potion otomatis saat bertarung
int usePotionInCombat(struct Player *p) {
    if (p->hp >= p->max_hp) {
        printf(">> HP sudah penuh!\n");
        return 0;
    }

    int indeks_potion = -1;
    for (int i = 0; i < p->item_count; i++) {
        if (p->inventory[i].type == ITEM_POTION && p->inventory[i].quantity > 0) {
            indeks_potion = i;
            break;
        }
    }

    if (indeks_potion == -1) {
        printf(">> Kamu tidak punya Potion!\n");
        return 0;
    }

    struct Item *pot = &p->inventory[indeks_potion];
    int hp_lama = p->hp;

    p->hp += pot->stat_value;
    if (p->hp > p->max_hp) {
        p->hp = p->max_hp;
    }

    printf(">> %s menggunakan %s! HP pulih +%d (%d/%d).\n",
           p->name, pot->name, p->hp - hp_lama, p->hp, p->max_hp);

    pot->quantity--;
    if (pot->quantity <= 0) {
        removeInventoryItem(p, indeks_potion);
    }

    return 1;
}