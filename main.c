#include "game.h"

void clearScreen() {
    system("cls || clear");
}

void pauseScreen() {
    printf("\nTekan [ENTER] untuk melanjutkan...");
    getchar();
}

// Validasi input angka agar program tidak error kalau user ketik huruf
int getValidIntInput(int min, int max) {
    int input;
    char ch;

    while (1) {
        if (scanf("%d", &input) == 1) {
            ch = getchar();
            if (ch == '\n') {
                if (input >= min && input <= max) {
                    return input;
                }
            } else {
                // Buang sisa karakter jika input tercampur huruf (misal: 1abc)
                while (getchar() != '\n');
            }
        } else {
            // Buang input jika bukan angka
            while (getchar() != '\n');
        }
        printf("Input tidak valid! Masukkan angka antara %d - %d: ", min, max);
    }
}

// Loop menu utama saat sedang bermain
void runGameLoop(struct Player *hero) {
    int bermain = 1;

    while (bermain == 1) {
        if (hero->hp <= 0) {
            printf("\n>> Petualangan %s telah berakhir. Kembali ke Menu Awal...\n", hero->name);
            pauseScreen();
            break;
        }

        clearScreen();
        printf("==================================================\n");
        printf("              PETUALANGAN TEXT RPG                \n");
        printf("==================================================\n");
        printf(" %s (%s) | Lv.%d | HP: %d/%d | Gold: %d G\n",
            hero->name, hero->class_type, hero->level,
            hero->hp, hero->max_hp, hero->gold);
        printf("==================================================\n");
        printf(" 1. Jelajah Dunia (Explore)\n");
        printf(" 2. Buka Tas (Inventory)\n");
        printf(" 3. Kunjungi Toko (Shop)\n");
        printf(" 4. Cek Status Karakter\n");
        printf(" 5. Istirahat di Desa (Pulihkan HP)\n");
        printf(" 6. Simpan Permainan (Save Game)\n");
        printf(" 7. Keluar ke Menu Awal\n");
        printf("==================================================\n");
        printf("Pilih aksi (1-7): ");

        int pilihan = getValidIntInput(1, 7);

        switch (pilihan) {
            case 1:
                exploreWorld(hero);
                break;
            case 2:
                openInventory(hero);
                break;
            case 3:
                openShop(hero);
                break;
            case 4:
                showStatus(*hero);
                break;
            case 5:
                restAtVillage(hero);
                break;
            case 6:
                if (saveGame(*hero) == 1) {
                    printf("\n>> Progres permainan berhasil disimpan ke '%s'!\n", SAVE_FILE);
                }
                pauseScreen();
                break;
            case 7:
                printf("\nPastikan kamu sudah Save sebelum keluar! Kembali ke Menu Awal...\n");
                pauseScreen();
                bermain = 0;
                break;
        }
    }
}

int main() {
    srand(time(NULL));

    struct Player hero;
    int program_jalan = 1;

    while (program_jalan == 1) {
        clearScreen();
        printf("==================================================\n");
        printf("         CHRONICLES OF ALGORITMA (MINI RPG)       \n");
        printf("       Project Akhir Pemrograman Dasar (C)        \n");
        printf("==================================================\n");
        printf(" 1. New Game (Mulai Petualangan Baru)\n");
        printf(" 2. Load Game (Lanjutkan dari Save)\n");
        printf(" 3. Exit (Keluar Program)\n");
        printf("==================================================\n");
        printf("Pilihan (1-3): ");

        int menu = getValidIntInput(1, 3);

        switch (menu) {
            case 1:
                createCharacter(&hero);
                runGameLoop(&hero);
                break;

            case 2:
                if (loadGame(&hero) == 1) {
                    printf("\n>> Save file ditemukan! Selamat datang kembali, %s (Lv.%d)!\n",
                        hero.name, hero.level);
                    pauseScreen();
                    runGameLoop(&hero);
                } else {
                    printf("\n>> File '%s' tidak ditemukan atau rusak! Silakan mulai New Game.\n",
                        SAVE_FILE);
                    pauseScreen();
                }
                break;

            case 3:
                clearScreen();
                printf("Terima kasih telah bermain! Sampai jumpa di Semester 3!\n");
                program_jalan = 0;
                break;
        }
    }

    return 0;
}