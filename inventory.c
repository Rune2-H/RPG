#include "game.h"

// ============================================================================
// MODUL 3: inventory.c (Toko, Tas, Pemakaian Item & Equipment)
// ============================================================================

// Daftar katalog barang Toko (statis di modul ini)
static struct Item shopCatalog[] = {
    {101, "Small Health Potion", ITEM_POTION, 40,  25,  1},
    {102, "Mega Health Potion",  ITEM_POTION, 100, 60,  1},
    {201, "Iron Broadsword",     ITEM_WEAPON, 8,   120, 1},
    {202, "Steel Claymore",      ITEM_WEAPON, 16,  280, 1},
    {301, "Leather Tunic",       ITEM_ARMOR,  5,   100, 1},
    {302, "Knight Chainmail",    ITEM_ARMOR,  12,  250, 1}
};
static const int SHOP_ITEM_COUNT = 6;

// Helper internal: Menghapus item pada indeks tertentu & menggeser array ke kiri
static void removeInventoryItem(struct Player *p, int index) {
    if (index < 0 || index >= p->item_count) return;

    for (int i = index; i < p->item_count - 1; i++) {
        p->inventory[i] = p->inventory[i + 1];
    }
    p->item_count--;
}

// Helper internal: Menambahkan item ke tas (stacking jika ID sudah ada)
static int addItemToInventory(struct Player *p, struct Item new_item) {
    // 1. Cek apakah item dengan item_id yang sama sudah ada di tas
    for (int i = 0; i < p->item_count; i++) {
        if (p->inventory[i].item_id == new_item.item_id) {
            p->inventory[i].quantity += 1;
            return 1; // Berhasil ditumpuk
        }
    }

    // 2. Jika belum ada, cek apakah slot tas masih muat
    if (p->item_count >= MAX_INVENTORY) {
        return 0; // Tas penuh
    }

    new_item.quantity = 1;
    p->inventory[p->item_count] = new_item;
    p->item_count++;
    return 1;
}

void openShop(struct Player *p) {
    while (1) {
        clearScreen();
        printf("==========================================================\n");
        printf("                     TOKO PETUALANG                       \n");
        printf("  Gold Kamu: %d G | Slot Tas: %d / %d\n", p->gold, p->item_count, MAX_INVENTORY);
        printf("==========================================================\n");
        printf(" No | Nama Item            | Tipe   | Efek        | Harga \n");
        printf("----------------------------------------------------------\n");

        for (int i = 0; i < SHOP_ITEM_COUNT; i++) {
            const char *type_str = (shopCatalog[i].type == ITEM_POTION) ? "Potion" :
                                   (shopCatalog[i].type == ITEM_WEAPON) ? "Weapon" : "Armor ";
            const char *stat_prefix = (shopCatalog[i].type == ITEM_POTION) ? "+HP " :
                                      (shopCatalog[i].type == ITEM_WEAPON) ? "+ATK" : "+DEF";

            printf(" %2d | %-20s | %s | %s %-6d | %4d G\n",
                   i + 1,
                   shopCatalog[i].name,
                   type_str,
                   stat_prefix,
                   shopCatalog[i].stat_value,
                   shopCatalog[i].price);
        }
        printf("----------------------------------------------------------\n");
        printf(" 0  | Keluar dari Toko\n");
        printf("==========================================================\n");
        printf("Pilih nomor item yang ingin dibeli (0-%d): ", SHOP_ITEM_COUNT);

        int choice = getValidIntInput(0, SHOP_ITEM_COUNT);
        if (choice == 0) {
            break;
        }

        struct Item selected = shopCatalog[choice - 1];

        if (p->gold < selected.price) {
            printf("\n>> Gold tidak cukup untuk membeli %s! (Kurang %d G)\n",
                   selected.name, selected.price - p->gold);
        } else {
            if (addItemToInventory(p, selected)) {
                p->gold -= selected.price;
                printf("\n>> Berhasil membeli %s seharga %d G!\n", selected.name, selected.price);
            } else {
                printf("\n>> Tas penuh! Tidak bisa menambah jenis item baru.\n");
            }
        }
        pauseScreen();
    }
}

void openInventory(struct Player *p) {
    while (1) {
        clearScreen();
        printf("==========================================================\n");
        printf("                    TAS / INVENTORY                       \n");
        printf("  HP: %d/%d | Bonus ATK: +%d | Bonus DEF: +%d\n",
               p->hp, p->max_hp, p->bonus_atk, p->bonus_def);
        printf("==========================================================\n");

        if (p->item_count == 0) {
            printf("  Tas masih kosong. Belanja dulu di Toko!\n");
            printf("==========================================================\n");
            pauseScreen();
            return;
        }

        printf(" No | Nama Item            | Tipe   | Efek     | Jml | Ket\n");
        printf("----------------------------------------------------------\n");

        for (int i = 0; i < p->item_count; i++) {
            struct Item it = p->inventory[i];
            const char *type_str = (it.type == ITEM_POTION) ? "Potion" :
                                   (it.type == ITEM_WEAPON) ? "Weapon" : "Armor ";
            const char *equipped_tag = "";

            if ((it.type == ITEM_WEAPON && p->equipped_weapon_id == it.item_id) ||
                (it.type == ITEM_ARMOR  && p->equipped_armor_id  == it.item_id)) {
                equipped_tag = "[E]";
            }

            printf(" %2d | %-20s | %s | +%-7d | %3d | %s\n",
                   i + 1, it.name, type_str, it.stat_value, it.quantity, equipped_tag);
        }
        printf("----------------------------------------------------------\n");
        printf(" 0  | Tutup Tas\n");
        printf("==========================================================\n");
        printf("Pilih nomor item untuk digunakan / dipasang (0-%d): ", p->item_count);

        int choice = getValidIntInput(0, p->item_count);
        if (choice == 0) {
            break;
        }

        int idx = choice - 1;
        struct Item *chosen = &p->inventory[idx];

        if (chosen->type == ITEM_POTION) {
            if (p->hp >= p->max_hp) {
                printf("\n>> HP sudah penuh (%d/%d)! Potion tidak dipakai.\n", p->hp, p->max_hp);
            } else {
                int old_hp = p->hp;
                p->hp += chosen->stat_value;
                if (p->hp > p->max_hp) p->hp = p->max_hp;

                printf("\n>> Menggunakan %s! HP pulih +%d (%d -> %d).\n",
                       chosen->name, p->hp - old_hp, old_hp, p->hp);

                chosen->quantity--;
                if (chosen->quantity <= 0) {
                    removeInventoryItem(p, idx); // Geser array jika habis
                }
            }
        } else if (chosen->type == ITEM_WEAPON) {
            if (p->equipped_weapon_id == chosen->item_id) {
                printf("\n>> %s sudah sedang terpasang!\n", chosen->name);
            } else {
                p->equipped_weapon_id = chosen->item_id;
                p->bonus_atk = chosen->stat_value;
                printf("\n>> Memasang %s! Bonus ATK sekarang +%d.\n",
                       chosen->name, p->bonus_atk);
            }
        } else if (chosen->type == ITEM_ARMOR) {
            if (p->equipped_armor_id == chosen->item_id) {
                printf("\n>> %s sudah sedang terpasang!\n", chosen->name);
            } else {
                p->equipped_armor_id = chosen->item_id;
                p->bonus_def = chosen->stat_value;
                printf("\n>> Memasang %s! Bonus DEF sekarang +%d.\n",
                       chosen->name, p->bonus_def);
            }
        }
        pauseScreen();
    }
}

// Dipanggil langsung dari combat.c saat Player memilih aksi "3. Gunakan Potion"
int usePotionInCombat(struct Player *p) {
    if (p->hp >= p->max_hp) {
        printf(">> HP sudah penuh! Tidak perlu menggunakan Potion.\n");
        return 0; // Gagal pakai (tidak membuang giliran)
    }

    // Cari potion pertama yang tersedia di tas
    int potion_idx = -1;
    for (int i = 0; i < p->item_count; i++) {
        if (p->inventory[i].type == ITEM_POTION && p->inventory[i].quantity > 0) {
            potion_idx = i;
            break;
        }
    }

    if (potion_idx == -1) {
        printf(">> Kamu tidak punya Potion di dalam tas!\n");
        return 0; // Gagal pakai (tidak membuang giliran)
    }

    struct Item *pot = &p->inventory[potion_idx];
    int old_hp = p->hp;
    p->hp += pot->stat_value;
    if (p->hp > p->max_hp) p->hp = p->max_hp;

    printf(">> %s meminum %s! HP pulih +%d (%d/%d).\n",
           p->name, pot->name, p->hp - old_hp, p->hp, p->max_hp);

    pot->quantity--;
    if (pot->quantity <= 0) {
        removeInventoryItem(p, potion_idx);
    }

    return 1; // Berhasil pakai (giliran terpakai)
}