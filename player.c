#include "game.h"

// ============================================================================
// MODUL 1: player.c (Karakter & Progresi)
// ============================================================================

void createCharacter(struct Player *p) {
    clearScreen();
    printf("==========================================\n");
    printf("          PEMBUATAN KARAKTER BARU         \n");
    printf("==========================================\n");

    // 1. Input Nama Karakter
    printf("Masukkan nama pahlawanmu: ");
    fgets(p->name, MAX_NAME_LEN, stdin);
    // Menghapus karakter newline (\n) bawaan fgets
    p->name[strcspn(p->name, "\n")] = '\0';

    // Validasi jika user hanya menekan Enter (nama kosong)
    if (strlen(p->name) == 0) {
        strcpy(p->name, "Petualang");
    }

    // 2. Pilih Class Karakter
    printf("\nPilih Class Karakter:\n");
    printf("1. Warrior  (HP: 120 | ATK: 15 | DEF: 10) - Kuat & Tahan Banting\n");
    printf("2. Mage     (HP: 80  | ATK: 25 | DEF: 5)  - Serangan Tinggi, HP Tipis\n");
    printf("3. Rogue    (HP: 100 | ATK: 20 | DEF: 7)  - Seimbang & Lincah\n");
    printf("Pilihan (1-3): ");

    int choice = getValidIntInput(1, 3);

    // 3. Inisialisasi Data Dasar & Progresi
    p->player_id = 1;
    p->level = 1;
    p->exp = 0;
    p->gold = 100; // Modal awal untuk belanja di Toko

    // Inisialisasi Equipment & Tas (Kosong di awal)
    p->equipped_weapon_id = 0;
    p->equipped_armor_id = 0;
    p->bonus_atk = 0;
    p->bonus_def = 0;
    p->item_count = 0;

    // Set Stat berdasarkan Class yang dipilih
    if (choice == 1) {
        strcpy(p->class_type, "Warrior");
        p->max_hp = 120;
        p->base_atk = 15;
        p->base_def = 10;
    } else if (choice == 2) {
        strcpy(p->class_type, "Mage");
        p->max_hp = 80;
        p->base_atk = 25;
        p->base_def = 5;
    } else {
        strcpy(p->class_type, "Rogue");
        p->max_hp = 100;
        p->base_atk = 20;
        p->base_def = 7;
    }

    p->hp = p->max_hp; // HP penuh saat baru dibuat

    printf("\nKarakter '%s' (%s) berhasil dibuat!\n", p->name, p->class_type);
    pauseScreen();
}

void showStatus(struct Player p) {
    clearScreen();
    int total_atk = p.base_atk + p.bonus_atk;
    int total_def = p.base_def + p.bonus_def;
    int exp_needed = p.level * 100;

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
    printf(" Weapon ID  : %s\n", p.equipped_weapon_id == 0 ? "Tidak ada" : "Terpasang");
    printf(" Armor ID   : %s\n", p.equipped_armor_id == 0 ? "Tidak ada" : "Terpasang");
    printf(" Isi Tas    : %d / %d slot terisi\n", p.item_count, MAX_INVENTORY);
    printf("==========================================\n");
    pauseScreen();
}

void checkLevelUp(struct Player *p) {
    int exp_needed = p->level * 100;
    int leveled_up = 0;

    // Menggunakan while agar jika EXP berlebih drastis (misal habis lawan Boss),
    // karakter bisa naik beberapa level sekaligus
    while (p->exp >= exp_needed) {
        p->exp -= exp_needed;
        p->level++;
        leveled_up = 1;

        // Kenaikan stat menyesuaikan Class
        if (strcmp(p->class_type, "Warrior") == 0) {
            p->max_hp += 25;
            p->base_atk += 4;
            p->base_def += 3;
        } else if (strcmp(p->class_type, "Mage") == 0) {
            p->max_hp += 15;
            p->base_atk += 7;
            p->base_def += 2;
        } else { // Rogue
            p->max_hp += 20;
            p->base_atk += 5;
            p->base_def += 2;
        }

        // Pulihkan HP penuh sebagai hadiah naik level
        p->hp = p->max_hp;
        exp_needed = p->level * 100;
    }

    if (leveled_up) {
        printf("\n******************************************\n");
        printf("  SELAMAT! %s NAIK KE LEVEL %d!\n", p->name, p->level);
        printf("  HP Pulih Penuh (%d/%d) & Stat Meningkat!\n", p->hp, p->max_hp);
        printf("******************************************\n");
    }
}