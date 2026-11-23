#include "game.h"

// ============================================================================
// MODUL 4: world.c (Peta, Jelajah Arah, Random Encounter & Istirahat)
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

// Helper internal: Memasukkan Potion gratis dari Chest ke tas player
static int giveChestPotion(struct Player *p, int is_mega) {
    struct Item potion;
    if (is_mega) {
        potion = (struct Item){102, "Mega Health Potion", ITEM_POTION, 100, 60, 1};
    } else {
        potion = (struct Item){101, "Small Health Potion", ITEM_POTION, 40, 25, 1};
    }

    // Cek apakah sudah ada di tas untuk ditumpuk (stack)
    for (int i = 0; i < p->item_count; i++) {
        if (p->inventory[i].item_id == potion.item_id) {
            p->inventory[i].quantity += 1;
            return 1;
        }
    }

    // Jika belum ada, taruh di slot baru jika muat
    if (p->item_count < MAX_INVENTORY) {
        p->inventory[p->item_count] = potion;
        p->item_count++;
        return 1;
    }
    return 0; // Tas penuh
}

// Helper internal: Sub-menu eksplorasi berdasarkan arah mata angin
static void exploreAreaByDirection(struct Player *p, int area_type) {
    const char *area_name = (area_type == 2) ? "Hutan Rimba" : "Gua Kegelapan";
    const char *dir_names[4] = {"Utara", "Selatan", "Timur", "Barat"};
    int steps = 0;

    while (p->hp > 0) {
        clearScreen();
        printf("==================================================\n");
        printf("  LOKASI: %-15s | Langkah ke-%d\n", area_name, steps);
        printf("  %s (Lv.%d) | HP: %d/%d | Gold: %d G\n",
               p->name, p->level, p->hp, p->max_hp, p->gold);
        printf("==================================================\n");
        printf("Pilih arah langkahmu:\n");
        printf(" 1. Melangkah ke Utara\n");
        printf(" 2. Melangkah ke Selatan\n");
        printf(" 3. Melangkah ke Timur\n");
        printf(" 4. Melangkah ke Barat\n");
        printf(" 0. Keluar dari %s (Kembali ke Peta Dunia)\n", area_name);
        printf("==================================================\n");
        printf("Pilihan arah (0-4): ");

        int dir = getValidIntInput(0, 4);
        if (dir == 0) {
            printf("\n>> Kamu berjalan keluar meninggalkan %s...\n", area_name);
            pauseScreen();
            break;
        }

        steps++;
        clearScreen();
        printf(">> Kamu melangkah menuju arah %s di dalam %s...\n\n",
               dir_names[dir - 1], area_name);

        int roll = rand() % 100; // Acak kejadian 0 - 99

        if (roll < 45) {
            // 45% Peluang: Ketemu Monster
            struct Monster enemy = (area_type == 2) ? spawnForestMonster() : spawnCaveMonster();
            printf("!! WASPADA !! Dari arah %s muncul %s (Lv.%d)!\n",
                   dir_names[dir - 1], enemy.name, enemy.level_req);
            pauseScreen();

            int result = startCombat(p, enemy, 1); // encounter biasa: boleh kabur
            if (result == COMBAT_LOSE) {
                return; // Kembali ke exploreWorld -> main.c untuk Game Over
            }

        } else if (roll < 70) {
            // 25% Peluang: Menemukan Treasure Chest (Bisa Gold atau Gold + Potion)
            int chest_type = rand() % 100;
            int found_gold = (area_type == 2) ? (15 + rand() % 26) : (40 + rand() % 51);
            p->gold += found_gold;

            printf("==================================================\n");
            printf("  [CHEST DITEMUKAN!] Kamu membuka peti harta karun!\n");
            printf("  >> Mendapatkan %d Gold! (Total: %d G)\n", found_gold, p->gold);

            if (chest_type < 45) {
                // 45% isi chest juga berisi Potion tambahan
                int is_mega = (area_type == 3);
                if (giveChestPotion(p, is_mega)) {
                    printf("  >> Bonus item: 1x %s masuk ke tas!\n",
                           is_mega ? "Mega Health Potion" : "Small Health Potion");
                } else {
                    printf("  >> Ada Potion di dalam peti, tapi tasmu penuh!\n");
                }
            }
            printf("==================================================\n");
            pauseScreen();

        } else if (roll < 80) {
            // 10% Peluang: Mata Air / Tempat Istirahat Singkat (Pulih sedikit HP)
            int heal_amt = (area_type == 2) ? 15 : 30;
            int old_hp = p->hp;
            p->hp += heal_amt;
            if (p->hp > p->max_hp) p->hp = p->max_hp;

            printf("++ Kamu menemukan mata air jernih di balik bebatuan.\n");
            printf("   Kamu meminumnya dan HP pulih +%d (%d -> %d).\n",
                   p->hp - old_hp, old_hp, p->hp);
            pauseScreen();

        } else {
            // 20% Peluang: Aman / Tidak ketemu apa-apa
            if (area_type == 2) {
                printf(".. Hanya suara angin dan dedaunan bergesekan. Jalur %s aman.\n",
                       dir_names[dir - 1]);
            } else {
                printf(".. Lorong gua di arah %s gelap dan sunyi. Tidak ada apa-apa di sini.\n",
                       dir_names[dir - 1]);
            }
            pauseScreen();
        }
    }
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

// ============================================================================
// MODE BOSS RUSH (INFINITE)
// ============================================================================

// Boss gelombang ke-`wave`; makin tinggi gelombang, makin kejam.
static struct Monster spawnBossRushMonster(int wave) {
    static const char *names[] = {
        "Malakor's Echo", "Crimson Revenant", "Ashen Warlord",
        "Void Herald", "Twin of the Abyss", "The Unnamed King"
    };

    struct Monster m;
    int idx = (wave - 1) % (int)(sizeof(names) / sizeof(names[0]));

    m.monster_id = 9000 + wave;
    strcpy(m.name, names[idx]);
    m.level_req = 5 + wave;
    m.max_hp = 260 + 70 * (wave - 1);
    m.hp = m.max_hp;
    m.atk = 30 + 5 * (wave - 1);
    m.def = 14 + 2 * (wave - 1);
    m.gold_reward = 150 + 50 * (wave - 1);
    m.exp_reward = 100 + 40 * (wave - 1);

    return m;
}

void startBossRush(struct Player *p) {
    clearScreen();
    printf("==================================================\n");
    printf("            INFINITE BOSS RUSH                    \n");
    printf("==================================================\n");
    printf("  Arena tanpa jalan pulang. Gelombang boss tiada\n");
    printf("  akhir: tidak ada toko, tidak ada istirahat, dan\n");
    printf("  tombol kabur terkunci. Hanya bekal yang sekarang\n");
    printf("  kamu bawa yang bisa menolongmu.\n");
    printf("--------------------------------------------------\n");
    printf("  !! PERINGATAN !!\n");
    printf("  Kalau kamu mati di dalam sini, karakter ini\n");
    printf("  DIHAPUS PERMANEN dan file save-nya ikut hilang.\n");
    printf("  Tidak ada kesempatan kedua. Pastikan sudah Save\n");
    printf("  kalau kamu masih ingin menyimpan progres lain.\n");
    printf("--------------------------------------------------\n");
    printf(" 1. Aku siap. Masuk arena.\n");
    printf(" 2. Batal (kembali ke Peta Dunia)\n");
    printf("Pilihan (1-2): ");

    int confirm = getValidIntInput(1, 2);
    if (confirm != 1) {
        return;
    }

    int wave = 0;

    while (1) {
        wave++;

        struct Monster boss = spawnBossRushMonster(wave);

        clearScreen();
        printf("==================================================\n");
        printf("  GELOMBANG %d: %s muncul!\n", wave, boss.name);
        printf("==================================================\n");
        pauseScreen();

        int result = startCombat(p, boss, 0); // 0 = kabur dikunci

        if (result == COMBAT_LOSE) {
            // Karakter gugur: save dihapus, tidak ada yang tersisa
            clearScreen();
            printf("==================================================\n");
            printf("         KARAKTER DIHAPUS PERMANEN                 \n");
            printf("==================================================\n");
            printf("  Kamu bertahan sampai gelombang %d.\n", wave);
            printf("  Save game '%s' dihapus. Nama %s hilang selamanya.\n",
                   SAVE_FILE, p->name);
            printf("==================================================\n");

            remove(SAVE_FILE);

            p->hp = 0;
            p->gold = 0;
            p->item_count = 0;
            p->boss_rush_unlocked = 0;

            pauseScreen();
            return;
        }

        printf("\n>> Gelombang %d selesai! Lanjut ke gelombang %d...\n", wave, wave + 1);
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
        printf(" 2. Hutan Rimba     [Lv. 1 - 2]  - Jelajah Arah, Chest & Monster Ringan\n");
        printf(" 3. Gua Kegelapan   [Lv. 3 - 4]  - Jelajah Arah, Chest & Monster Kuat\n");
        printf(" 4. Kastil Boss     [Lv. 5+]     - Lawan Raja Iblis (Final Boss)\n");
        if (p->boss_rush_unlocked) {
            printf(" 5. BOSS RUSH       [MODE BARU]  - Gelombang boss tanpa akhir (mati = hapus)\n");
        }
        printf(" 0. Kembali ke Menu Utama\n");
        printf("==================================================\n");

        if (p->boss_rush_unlocked) {
            printf("Pilih tujuan jelajah (0-5): ");
        } else {
            printf("Pilih tujuan jelajah (0-4): ");
        }

        int max_choice = p->boss_rush_unlocked ? 5 : 4;
        int choice = getValidIntInput(0, max_choice);
        if (choice == 0) {
            break;
        }

        if (choice == 1) {
            restAtVillage(p);
        } else if (choice == 2 || choice == 3) {
            // Masuk ke mode jelajah arah (Utara/Selatan/Timur/Barat)
            exploreAreaByDirection(p, choice);
        } else if (choice == 5 && p->boss_rush_unlocked) {
            startBossRush(p);
        } else if (choice == 4) {
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
                int result = startCombat(p, boss, 1);

                if (result == COMBAT_WIN) {
                    p->boss_rush_unlocked = 1;

                    clearScreen();
                    printf("**************************************************\n");
                    printf("   SELAMAT! KAMU MENAMATKAN CHRONICLES OF ALGORITMA!\n");
                    printf("   %s (Lv.%d) resmi menjadi Pahlawan Legendaris!\n",
                           p->name, p->level);
                    printf("**************************************************\n");
                    printf("\n");
                    printf("  >>> MODE BARU TERBUKA: INFINITE BOSS RUSH <<<\n");
                    printf("  Gelombang boss tanpa akhir, tanpa toko, tanpa\n");
                    printf("  kabur. Masuk lewat Peta Dunia -> pilihan 5.\n");
                    printf("  PERINGATAN: kalau kamu mati di sana, karakter\n");
                    printf("  ini dihapus permanen. Save dulu kalau ragu!\n");
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