#include "game.h"

// ============================================================================
// MODUL 5: saveload.c (Save & Load Data via File Handling)
// ============================================================================

int saveGame(struct Player p) {
    FILE *fp = fopen(SAVE_FILE, "w");
    if (fp == NULL) {
        printf("\n>> [ERROR] Gagal membuka file '%s' untuk menyimpan!\n", SAVE_FILE);
        return 0;
    }

    // 1. Tulis Identitas Karakter (menggunakan titik koma ';' agar nama ber-spasi aman)
    fprintf(fp, "%d;%s;%s\n", p.player_id, p.name, p.class_type);

    // 2. Tulis Statistik & Progresi
    fprintf(fp, "%d %d %d %d %d %d %d\n",
            p.level, p.exp, p.hp, p.max_hp, p.base_atk, p.base_def, p.gold);

    // 3. Tulis Status Equipment & Jumlah Item di Tas
    fprintf(fp, "%d %d %d %d %d\n",
            p.equipped_weapon_id, p.equipped_armor_id,
            p.bonus_atk, p.bonus_def, p.item_count);

    // 4. Tulis seluruh isi Inventory (1 baris per item)
    for (int i = 0; i < p.item_count; i++) {
        fprintf(fp, "%d;%s;%d;%d;%d;%d\n",
                p.inventory[i].item_id,
                p.inventory[i].name,
                p.inventory[i].type,
                p.inventory[i].stat_value,
                p.inventory[i].price,
                p.inventory[i].quantity);
    }

    fclose(fp);
    return 1;
}

int loadGame(struct Player *p) {
    FILE *fp = fopen(SAVE_FILE, "r");
    if (fp == NULL) {
        return 0; // File savegame.txt belum ada
    }

    // 1. Baca Identitas Karakter (scanset %[^;] membaca teks sampai ketemu ';')
    if (fscanf(fp, "%d;%49[^;];%19[^\n]\n",
               &p->player_id, p->name, p->class_type) != 3) {
        fclose(fp);
        return 0; // Format file rusak
    }

    // 2. Baca Statistik & Progresi
    if (fscanf(fp, "%d %d %d %d %d %d %d\n",
               &p->level, &p->exp, &p->hp, &p->max_hp,
               &p->base_atk, &p->base_def, &p->gold) != 7) {
        fclose(fp);
        return 0;
    }

    // 3. Baca Status Equipment & Jumlah Item di Tas
    if (fscanf(fp, "%d %d %d %d %d\n",
               &p->equipped_weapon_id, &p->equipped_armor_id,
               &p->bonus_atk, &p->bonus_def, &p->item_count) != 5) {
        fclose(fp);
        return 0;
    }

    // Pengaman agar tidak melebihi kapasitas array
    if (p->item_count > MAX_INVENTORY) {
        p->item_count = MAX_INVENTORY;
    }

    // 4. Baca seluruh isi Inventory
    for (int i = 0; i < p->item_count; i++) {
        if (fscanf(fp, "%d;%49[^;];%d;%d;%d;%d\n",
                   &p->inventory[i].item_id,
                   p->inventory[i].name,
                   &p->inventory[i].type,
                   &p->inventory[i].stat_value,
                   &p->inventory[i].price,
                   &p->inventory[i].quantity) != 6) {
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;
}