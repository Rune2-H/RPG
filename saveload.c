#include "game.h"

// Fungsi bantuan untuk membersihkan \r dan \n di akhir string
void trimNewline(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

int saveGame(struct Player p) {
    FILE *fp = fopen(SAVE_FILE, "w");
    if (fp == NULL) {
        printf("\n>> [ERROR] Gagal membuka file '%s' untuk menyimpan!\n", SAVE_FILE);
        return 0;
    }

    // Pastikan item_count valid sebelum disimpan agar tidak out-of-bounds
    if (p.item_count < 0) p.item_count = 0;
    if (p.item_count > MAX_INVENTORY) p.item_count = MAX_INVENTORY;

    // Pastikan nama/class tidak kosong agar strtok saat load tidak gagal
    const char *safe_name = (strlen(p.name) > 0) ? p.name : "Player";
    const char *safe_class = (strlen(p.class_type) > 0) ? p.class_type : "Warrior";

    // 1. Simpan identitas karakter
    fprintf(fp, "%d;%s;%s\n", p.player_id, safe_name, safe_class);

    // 2. Simpan stat utama dan gold
    fprintf(fp, "%d %d %d %d %d %d %d\n",
            p.level, p.exp, p.hp, p.max_hp, p.base_atk, p.base_def, p.gold);

    // 3. Simpan equipment yang dipakai dan jumlah item
    fprintf(fp, "%d %d %d %d %d\n",
            p.equipped_weapon_id, p.equipped_armor_id,
            p.bonus_atk, p.bonus_def, p.item_count);

    // 4. Simpan semua item di tas
    for (int i = 0; i < p.item_count; i++) {
        const char *item_name = (strlen(p.inventory[i].name) > 0) ? p.inventory[i].name : "Item";
        fprintf(fp, "%d;%s;%d;%d;%d;%d\n",
                p.inventory[i].item_id,
                item_name,
                p.inventory[i].type,
                p.inventory[i].stat_value,
                p.inventory[i].price,
                p.inventory[i].quantity);
    }

    // 5. Simpan status mode Boss Rush
    fprintf(fp, "%d\n", p.boss_rush_unlocked);

    fclose(fp);
    return 1;
}

int loadGame(struct Player *p) {
    if (p == NULL) return 0;

    FILE *fp = fopen(SAVE_FILE, "r");
    if (fp == NULL) {
        return 0;
    }

    char baris[256];

    // 1. Baca baris identitas karakter (id;nama;class)
    if (fgets(baris, sizeof(baris), fp) == NULL) {
        fclose(fp);
        return 0;
    }
    trimNewline(baris);

    char *potong = strtok(baris, ";");
    if (potong == NULL) { fclose(fp); return 0; }
    p->player_id = atoi(potong);

    potong = strtok(NULL, ";");
    if (potong == NULL) { fclose(fp); return 0; }
    strncpy(p->name, potong, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';

    potong = strtok(NULL, ";");
    if (potong == NULL) { fclose(fp); return 0; }
    strncpy(p->class_type, potong, sizeof(p->class_type) - 1);
    p->class_type[sizeof(p->class_type) - 1] = '\0';

    // 2. Baca statistik dan progress menggunakan fgets + sscanf (lebih aman dari fscanf)
    if (fgets(baris, sizeof(baris), fp) == NULL ||
        sscanf(baris, "%d %d %d %d %d %d %d",
               &p->level, &p->exp, &p->hp, &p->max_hp,
               &p->base_atk, &p->base_def, &p->gold) != 7) {
        fclose(fp);
        return 0;
    }

    // 3. Baca equipment dan jumlah item di tas
    int total_item_di_file = 0;
    if (fgets(baris, sizeof(baris), fp) == NULL ||
        sscanf(baris, "%d %d %d %d %d",
               &p->equipped_weapon_id, &p->equipped_armor_id,
               &p->bonus_atk, &p->bonus_def, &total_item_di_file) != 5) {
        fclose(fp);
        return 0;
    }

    if (total_item_di_file < 0) total_item_di_file = 0;
    p->item_count = (total_item_di_file > MAX_INVENTORY) ? MAX_INVENTORY : total_item_di_file;

    // 4. Baca daftar item di inventory
    for (int i = 0; i < total_item_di_file; i++) {
        if (fgets(baris, sizeof(baris), fp) == NULL) {
            fclose(fp);
            return 0;
        }
        // Jika di file ada lebih dari MAX_INVENTORY, tetap baca barisnya tapi jangan simpan ke array
        if (i >= MAX_INVENTORY) continue;

        trimNewline(baris);

        potong = strtok(baris, ";");
        if (potong == NULL) { fclose(fp); return 0; }
        p->inventory[i].item_id = atoi(potong);

        potong = strtok(NULL, ";");
        if (potong == NULL) { fclose(fp); return 0; }
        strncpy(p->inventory[i].name, potong, sizeof(p->inventory[i].name) - 1);
        p->inventory[i].name[sizeof(p->inventory[i].name) - 1] = '\0';

        potong = strtok(NULL, ";");
        if (potong == NULL) { fclose(fp); return 0; }
        p->inventory[i].type = atoi(potong);

        potong = strtok(NULL, ";");
        if (potong == NULL) { fclose(fp); return 0; }
        p->inventory[i].stat_value = atoi(potong);

        potong = strtok(NULL, ";");
        if (potong == NULL) { fclose(fp); return 0; }
        p->inventory[i].price = atoi(potong);

        potong = strtok(NULL, ";");
        if (potong == NULL) { fclose(fp); return 0; }
        p->inventory[i].quantity = atoi(potong);
    }

    // 5. Baca status Boss Rush
    p->boss_rush_unlocked = 0;
    if (fgets(baris, sizeof(baris), fp) != NULL) {
        sscanf(baris, "%d", &p->boss_rush_unlocked);
    }

    fclose(fp);
    return 1;
}