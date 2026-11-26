#include "game.h"

void createCharacter(struct Player *p) {
    clearScreen();
    printf("==========================================\n");
    printf("          PEMBUATAN KARAKTER BARU         \n");
    printf("==========================================\n");

    printf("Masukkan nama pahlawanmu: ");
    fgets(p->name, MAX_NAME_LEN, stdin);

    // Hapus karakter enter (\n) di akhir input fgets secara manual
    int panjang = strlen(p->name);
    if (panjang > 0 && p->name[panjang - 1] == '\n') {
        p->name[panjang - 1] = '\0';
    }

    // Jika user langsung tekan enter tanpa isi nama
    if (strlen(p->name) == 0) {
        strcpy(p->name, "Petualang");
    }

    printf("\nPilih Class Karakter:\n");
    printf("1. Warrior  (HP: 150 | ATK: 15 | DEF: 10) - Kuat & Tahan Banting\n");
    printf("2. Mage     (HP: 80  | ATK: 25 | DEF: 6)  - Serangan Tinggi, HP Tipis\n");
    printf("3. Rogue    (HP: 100 | ATK: 20 | DEF: 7)  - Seimbang & Lincah\n");
    printf("Pilihan (1-3): ");
    int pilihan = getValidIntInput(1, 3);

    // Data awal karakter
    p->player_id = 1;
    p->level = 1;
    p->exp = 0;
    p->gold = 100;
    p->boss_rush_unlocked = 0;

    // Atur stat dasar dan senjata awal sesuai class
    if (pilihan == 1) {
        strcpy(p->class_type, "Warrior");
        p->max_hp = 150;
        p->base_atk = 15;
        p->base_def = 10;

        p->inventory[0].item_id = 211;
        strcpy(p->inventory[0].name, "Rusty Sword");
        p->inventory[0].stat_value = 4;
    } else if (pilihan == 2) {
        strcpy(p->class_type, "Mage");
        p->max_hp = 80;
        p->base_atk = 25;
        p->base_def = 6;

        p->inventory[0].item_id = 212;
        strcpy(p->inventory[0].name, "Apprentice Staff");
        p->inventory[0].stat_value = 5;
    } else {
        strcpy(p->class_type, "Rogue");
        p->max_hp = 100;
        p->base_atk = 20;
        p->base_def = 7;

        p->inventory[0].item_id = 213;
        strcpy(p->inventory[0].name, "Worn Dagger");
        p->inventory[0].stat_value = 4;
    }

    // Properti umum senjata awal (Slot 0)
    p->inventory[0].type = ITEM_WEAPON;
    p->inventory[0].price = 20;
    p->inventory[0].quantity = 1;

    // Armor awal (Slot 1)
    p->inventory[1].item_id = 311;
    strcpy(p->inventory[1].name, "Cloth Tunic");
    p->inventory[1].type = ITEM_ARMOR;
    p->inventory[1].stat_value = 2;
    p->inventory[1].price = 20;
    p->inventory[1].quantity = 1;

    // Potion awal (Slot 2)
    p->inventory[2].item_id = 101;
    strcpy(p->inventory[2].name, "Small Health Potion");
    p->inventory[2].type = ITEM_POTION;
    p->inventory[2].stat_value = 40;
    p->inventory[2].price = 15;
    p->inventory[2].quantity = 2;

    // Set jumlah tas & langsung pakai senjata dan armor awal
    p->item_count = 3;
    p->equipped_weapon_id = p->inventory[0].item_id;
    p->bonus_atk = p->inventory[0].stat_value;

    p->equipped_armor_id = p->inventory[1].item_id;
    p->bonus_def = p->inventory[1].stat_value;

    p->hp = p->max_hp;

    printf("\nKarakter '%s' (%s) berhasil dibuat!\n", p->name, p->class_type);
    printf("Bekal Awal: %s, Cloth Tunic, dan 2x Small Potion.\n", p->inventory[0].name);
    pauseScreen();
}

void showStatus(struct Player p) {
    clearScreen();
    int total_atk = p.base_atk + p.bonus_atk;
    int total_def = p.base_def + p.bonus_def;
    int exp_needed = p.level * 100;

    char nama_senjata[MAX_NAME_LEN] = "Tidak ada";
    char nama_armor[MAX_NAME_LEN] = "Tidak ada";

    // Cari nama equipment yang sedang dipakai berdasarkan ID-nya
    for (int i = 0; i < p.item_count; i++) {
        if (p.inventory[i].item_id == p.equipped_weapon_id) {
            strcpy(nama_senjata, p.inventory[i].name);
        }
        if (p.inventory[i].item_id == p.equipped_armor_id) {
            strcpy(nama_armor, p.inventory[i].name);
        }
    }

    printf("==========================================\n");
    printf("             STATUS KARAKTER              \n");
    printf("==========================================\n");
    printf(" Nama       : %s\n", p.name);
    printf(" Class      : %s\n", p.class_type);
    printf(" Level      : %d\n", p.level);
    printf(" EXP        : %d / %d\n", p.exp, exp_needed);
    printf("------------------------------------------\n");
    printf(" HP         : %d / %d\n", p.hp, p.max_hp);
    printf(" Total ATK  : %d (Base: %d + Weapon: %d)\n", total_atk, p.base_atk, p.bonus_atk);
    printf(" Total DEF  : %d (Base: %d + Armor: %d)\n", total_def, p.base_def, p.bonus_def);
    printf(" Gold       : %d G\n", p.gold);
    printf("------------------------------------------\n");
    printf(" Weapon     : %s\n", nama_senjata);
    printf(" Armor      : %s\n", nama_armor);
    printf(" Isi Tas    : %d / %d slot terisi\n", p.item_count, MAX_INVENTORY);
    printf("==========================================\n");
    pauseScreen();
}

void checkLevelUp(struct Player *p) {
    int exp_needed = p->level * 100;
    int naik_level = 0;

    // Pakai while supaya kalau EXP berlebih banyak bisa naik beberapa level sekaligus
    while (p->exp >= exp_needed) {
        p->exp = p->exp - exp_needed;
        p->level++;
        naik_level = 1;

        if (strcmp(p->class_type, "Warrior") == 0) {
            p->max_hp += 25;
            p->base_atk += 4;
            p->base_def += 12;
        } else if (strcmp(p->class_type, "Mage") == 0) {
            p->max_hp += 15;
            p->base_atk += 7;
            p->base_def += 2;
        } else {
            p->max_hp += 20;
            p->base_atk += 5;
            p->base_def += 2;
        }

        p->hp = p->max_hp;
        exp_needed = p->level * 100;
    }

    if (naik_level == 1) {
        printf("\n******************************************\n");
        printf("  SELAMAT! %s NAIK KE LEVEL %d!\n", p->name, p->level);
        printf("  HP Pulih Penuh (%d/%d) & Stat Meningkat!\n", p->hp, p->max_hp);
        printf("******************************************\n");
    }
}