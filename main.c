#include "game.h"

// ============================================================================
// MODUL 6: main.c (Integrasi Seluruh Modul, UI Terminal & Validasi Input)
// ============================================================================

// --- IMPLEMENTASI HELPER UI & VALIDASI INPUT (Dideklarasikan di game.h) ---

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    printf("\nTekan [ENTER] untuk melanjutkan...");
    getchar();
}

int getValidIntInput(int min, int max) {
    int input;
    char ch;

    while (1) {
        if (scanf("%d", &input) == 1) {
            // Pastikan tidak ada karakter sampah setelah angka (misal: "1abc")
            ch = getchar();
            if (ch == '\n' || ch == EOF) {
                if (input >= min && input <= max) {
                    return input;
                }
            } else {
                // Bersihkan sisa karakter di buffer jika format salah
                while (getchar() != '\n');
            }
        } else {
            // Bersihkan buffer jika user mengetik huruf/simbol
            while (getchar() != '\n');
        }
        printf("Input tidak valid! Masukkan angka antara %d - %d: ", min, max);
    }
}

// --- ALUR MENU UTAMA DALAM GAME (GAME LOOP) ---

static void runGameLoop(struct Player *hero) {
    int playing = 1;

    while (playing) {
        // Cek apakah hero mati saat menjelajah / bertarung
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

        int choice = getValidIntInput(1, 7);

        switch (choice) {
            case 1:
                exploreWorld(hero);      // Modul 4: world.c
                break;
            case 2:
                openInventory(hero);     // Modul 3: inventory.c
                break;
            case 3:
                openShop(hero);          // Modul 3: inventory.c
                break;
            case 4:
                showStatus(*hero);       // Modul 1: player.c
                break;
            case 5:
                restAtVillage(hero);     // Modul 4: world.c
                break;
            case 6:
                if (saveGame(*hero)) {   // Modul 5: saveload.c
                    printf("\n>> Progres permainan berhasil disimpan ke '%s'!\n", SAVE_FILE);
                }
                pauseScreen();
                break;
            case 7:
                printf("\nPastikan kamu sudah Save sebelum keluar! Kembali ke Menu Awal...\n");
                pauseScreen();
                playing = 0;
                break;
        }
    }
}

// --- ENTRY POINT PROGRAM ---

int main() {
    // Inisialisasi seed random satu kali untuk seluruh game (combat & world)
    srand((unsigned int)time(NULL));

    struct Player hero;
    int app_running = 1;

    while (app_running) {
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

        int menu_choice = getValidIntInput(1, 3);

        switch (menu_choice) {
            case 1:
                createCharacter(&hero);  // Modul 1: player.c
                runGameLoop(&hero);
                break;

            case 2:
                if (loadGame(&hero)) {   // Modul 5: saveload.c
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
                app_running = 0;
                break;
        }
    }

    return 0;
}