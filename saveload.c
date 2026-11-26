#include "game.h"

int saveGame(struct Player p) {
    FILE *fp = fopen(SAVE_FILE, "w");
    if (fp == NULL) {
        printf("\n>> [ERROR] Gagal membuka file '%s' untuk menyimpan!\n", SAVE_FILE);
        return 0;
    }

    // Simpan identitas karakter (pakai titik koma biar nama yang ada spasi aman)
    fprintf(fp, "%d;%s;%s\n", p.player_id, p.name, p.class_type);

    // Simpan stat utama dan gold
    fprintf(fp, "%d %d %d %d %d %d %d\n",
            p.level, p.exp, p.hp, p.max_hp, p.base_atk, p.base_def, p.gold);

    // Simpan equipment yang dipakai dan jumlah item
    fprintf(fp, "%d %d %d %d %d\n",
            p.equipped_weapon_id, p.equipped_armor_id,
            p.bonus_atk, p.bonus_def, p.item_count);

    // Simpan semua item di tas
    for (int i = 0; i < p.item_count; i++) {
        fprintf(fp, "%d;%s;%d;%d;%d;%d\n",
                p.inventory[i].item_id,
                p.inventory[i].name,
                p.inventory[i].type,
                p.inventory[i].stat_value,
                p.inventory[i].price,
                p.inventory[i].quantity);
    }

    // Simpan status mode Boss Rush
    fprintf(fp, "%d\n", p.boss_rush_unlocked);

    fclose(fp);
    return 1;
}

int loadGame(struct Player *p) {
    FILE *fp = fopen(SAVE_FILE, "r");
    if (fp == NULL) {
        return 0;
    }

    char baris[200];

    // 1. Baca baris identitas karakter (id;nama;class)
    if (fgets(baris, sizeof(baris), fp) == NULL) {
        fclose(fp);
        return 0;
    }
    int panjang = strlen(baris);
    if (panjang > 0 && baris[panjang - 1] == '\n') {
        baris[panjang - 1] = '\0';
    }

    char *potong = strtok(baris, ";");
    if (potong == NULL) {
        fclose(fp);
        return 0;
    }
    p->player_id = atoi(potong);

    potong = strtok(NULL, ";");
    if (potong == NULL) {
        fclose(fp);
        return 0;
    }
    strcpy(p->name, potong);

    potong = strtok(NULL, ";");
    if (potong == NULL) {
        fclose(fp);
        return 0;
    }
    strcpy(p->class_type, potong);

    // 2. Baca statistik dan progress
    if (fscanf(fp, "%d %d %d %d %d %d %d\n",
               &p->level, &p->exp, &p->hp, &p->max_hp,
               &p->base_atk, &p->base_def, &p->gold) != 7) {
        fclose(fp);
        return 0;
    }

    // 3. Baca equipment dan jumlah item di tas
    if (fscanf(fp, "%d %d %d %d %d\n",
               &p->equipped_weapon_id, &p->equipped_armor_id,
               &p->bonus_atk, &p->bonus_def, &p->item_count) != 5) {
        fclose(fp);
        return 0;
    }

    if (p->item_count > MAX_INVENTORY) {
        p->item_count = MAX_INVENTORY;
    }

    // 4. Baca daftar item di inventory
    for (int i = 0; i < p->item_count; i++) {
        if (fgets(baris, sizeof(baris), fp) == NULL) {
            fclose(fp);
            return 0;
        }
        panjang = strlen(baris);
        if (panjang > 0 && baris[panjang - 1] == '\n') {
            baris[panjang - 1] = '\0';
        }

        potong = strtok(baris, ";");
        if (potong == NULL) { fclose(fp); return 0; }
        p->inventory[i].item_id = atoi(potong);

        potong = strtok(NULL, ";");
        if (potong == NULL) { fclose(fp); return 0; }
        strcpy(p->inventory[i].name, potong);

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

    // 5. Baca status Boss Rush (opsional untuk file save lama)
    p->boss_rush_unlocked = 0;
    fscanf(fp, "%d", &p->boss_rush_unlocked);

    fclose(fp);
    return 1;
}