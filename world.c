#include "game.h"

// ============================================================================
// MODUL 4: world.c (Peta, Jelajah Dunia, Random Encounter & Istirahat)
// ============================================================================

// Helper internal: Membuat monster berdasarkan tingkat bahaya lokasi
static struct Monster spawnForestMonster() {
    int pick = rand() % 2;
    if (pick == 0) {
        // {monster_id, name, level_req, hp, max_hp, atk, def, gold_reward, exp_reward}
        struct Monster m = {101, "Forest Slime", 1, 45, 45, 11, 2, 25, 35};
        return m;
    } else {
        struct Monster m = {102, "Wild Goblin", 2, 65, 65, 15, 4, 45, 55};
        return m;
    }
}

static struct Monster spawnCaveMonster() {
    int pick = rand() % 2;
    if (pick == 0) {
        struct Monster m = {201, "Skeleton Warrior", 3, 100, 100, 22, 8, 80, 95};
        return m;
    } else {
        struct Monster m = {202, "Cave Troll", 4, 140, 140, 28, 12, 120, 140};
        return m;
    }
}

static struct Monster spawnBossMonster() {
    struct Monster boss = {999, "Malakor, Raja Iblis", 6, 260, 260, 38, 16, 500, 400};
    return boss;
}

void restAtVillage(struct Player *p) {
    clearScreen();
    const int INN_COST = 20;

    printf("==================================================\n");
    printf("               PENGINAPAN DESA                    \n");
    printf("==================================================\n");
    printf(" HP Kamu saat ini : %d / %d\n", p->hp, p->max_hp);
    printf(" Gold Kamu        : %d G\n", p->gold);
    printf(" Biaya Istirahat  : %d G (Memulihkan HP Penuh)\n", INN_COST);
    printf("--------------------------------------------------\n");

    if (p->hp >= p->max_hp) {
        printf(">> HP kamu sudah penuh! Tidak perlu beristirahat.\n");
        pauseScreen();
        return;
    }

    printf("1. Istirahat sekarang (-%d G)\n", INN_COST);
    printf("2. Kembali\n");
    printf("Pilihan (1-2): ");

    int choice = getValidIntInput(1, 2);
    if (choice == 1) {
        if (p->gold < INN_COST) {
            printf("\n>> Gold tidak cukup! (Butuh %d G, kamu punya %d G)\n", INN_COST, p->gold);
        } else {
            p->gold -= INN_COST;
            p->hp = p->max_hp;
            printf("\n>> Kamu tidur nyenyak di penginapan. HP pulih penuh (%d/%d)!\n",
                   p->hp, p->max_hp);
        }
        pauseScreen();
    }
}

void exploreWorld(struct Player *p) {
    while (p->hp > 0) {
        clearScreen();
        printf("==================================================\n");
        printf("                  PETA DUNIA                      \n");
        printf("  %s (Lv.%d) | HP: %d/%d | Gold: %d G\n",
               p->name, p->level, p->hp, p->max_hp, p->gold);
        printf("==================================================\n");
        printf(" 1. Desa Awal       [Aman]       - Istirahat & Pulihkan HP\n");
        printf(" 2. Hutan Rimba     [Lv. 1 - 2]  - Monster Ringan & Harta\n");
        printf(" 3. Gua Kegelapan   [Lv. 3 - 4]  - Monster Kuat & Harta Besar\n");
        printf(" 4. Kastil Boss     [Lv. 5+]     - Lawan Raja Iblis (Final Boss)\n");
        printf(" 0. Kembali ke Menu Utama\n");
        printf("==================================================\n");
        printf("Pilih tujuan jelajah (0-4): ");

        int choice = getValidIntInput(0, 4);
        if (choice == 0) {
            break;
        }

        if (choice == 1) {
            restAtVillage(p);
            continue;
        }

        // --- AREA BERBAHAYA: HUTAN (2) & GUA (3) ---
        if (choice == 2 || choice == 3) {
            clearScreen();
            const char *loc_name = (choice == 2) ? "Hutan Rimba" : "Gua Kegelapan";
            printf(">> Kamu melangkah masuk ke dalam %s...\n\n", loc_name);

            int roll = rand() % 100; // Acak 0 - 99

            if (roll < 60) {
                // 60% Peluang: Random Encounter Monster
                struct Monster enemy = (choice == 2) ? spawnForestMonster() : spawnCaveMonster();
                printf("!! WASPADA !! Seekor %s muncul dari balik bayangan!\n", enemy.name);
                pauseScreen();

                int result = startCombat(p, enemy); // Panggil combat.c
                if (result == COMBAT_LOSE) {
                    return; // Keluar ke main.c untuk proses Game Over
                }

            } else if (roll < 85) {
                // 25% Peluang: Menemukan Harta Karun
                int found_gold = (choice == 2) ? (15 + rand() % 26) : (40 + rand() % 51);
                p->gold += found_gold;
                printf("$$ REZEKI NOMPLOK! Kamu menemukan peti tua berisi %d Gold!\n", found_gold);
                printf("   Total Gold sekarang: %d G\n", p->gold);
                pauseScreen();

            } else {
                // 15% Peluang: Aman
                printf(".. Suasana sekitar sunyi dan aman. Kamu tidak menemukan apa-apa.\n");
                pauseScreen();
            }
        }

        // --- AREA BOSS: KASTIL RAJA IBLIS (4) ---
        else if (choice == 4) {
            clearScreen();
            printf("==================================================\n");
            printf("  PERINGATAN: Kamu berada di depan gerbang Boss!\n");
            if (p->level < 5) {
                printf("  [Saran: Levelmu masih Lv.%d, disarankan minimal Lv.5]\n", p->level);
            }
            printf("==================================================\n");
            printf("1. Masuk dan Tantang Raja Iblis!\n");
            printf("2. Mundur dulu\n");
            printf("Pilihan (1-2): ");

            int confirm = getValidIntInput(1, 2);
            if (confirm == 1) {
                struct Monster boss = spawnBossMonster();
                int result = startCombat(p, boss);

                if (result == COMBAT_WIN) {
                    clearScreen();
                    printf("**************************************************\n");
                    printf("   SELAMAT! KAMU MENAMATKAN CHRONICLES OF ALGORITMA!\n");
                    printf("   %s (Lv.%d) resmi menjadi Pahlawan Legendaris!\n",
                           p->name, p->level);
                    printf("**************************************************\n");
                    printf("Kamu masih bisa lanjut menjelajah atau Save progresmu.\n");
                    pauseScreen();
                } else if (result == COMBAT_LOSE) {
                    return; // Kembali ke main.c untuk Game Over
                }
            }
        }
    }
}