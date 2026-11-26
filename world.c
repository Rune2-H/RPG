#include "game.h"

#define INN_COST 20
#define BOSS_RUSH_NAMES_COUNT 6

// Buat monster Hutan Rimba secara acak
struct Monster spawnForestMonster() {
    struct Monster m;
    int pilih = rand() % 2;

    if (pilih == 0) {
        m.monster_id = 101;
        strcpy(m.name, "Forest Slime");
        m.level_req = 1;
        m.max_hp = 45;
        m.hp = 45;
        m.atk = 11;
        m.def = 2;
        m.gold_reward = 25;
        m.exp_reward = 35;
    } else {
        m.monster_id = 102;
        strcpy(m.name, "Wild Goblin");
        m.level_req = 2;
        m.max_hp = 65;
        m.hp = 65;
        m.atk = 15;
        m.def = 4;
        m.gold_reward = 45;
        m.exp_reward = 55;
    }
    return m;
}

// Buat monster Gua Kegelapan secara acak
struct Monster spawnCaveMonster() {
    struct Monster m;
    int pilih = rand() % 2;

    if (pilih == 0) {
        m.monster_id = 201;
        strcpy(m.name, "Skeleton Warrior");
        m.level_req = 3;
        m.max_hp = 100;
        m.hp = 100;
        m.atk = 22;
        m.def = 8;
        m.gold_reward = 80;
        m.exp_reward = 95;
    } else {
        m.monster_id = 202;
        strcpy(m.name, "Cave Troll");
        m.level_req = 4;
        m.max_hp = 140;
        m.hp = 140;
        m.atk = 28;
        m.def = 12;
        m.gold_reward = 120;
        m.exp_reward = 140;
    }
    return m;
}

// Buat Final Boss
struct Monster spawnBossMonster() {
    struct Monster boss;
    boss.monster_id = 999;
    strcpy(boss.name, "Malakor, Raja Iblis");
    boss.level_req = 6;
    boss.max_hp = 260;
    boss.hp = 260;
    boss.atk = 38;
    boss.def = 16;
    boss.gold_reward = 500;
    boss.exp_reward = 400;
    return boss;
}

// Masukkan potion hadiah chest ke tas player
int giveChestPotion(struct Player *p, int is_mega) {
    struct Item potion;

    if (is_mega == 1) {
        potion.item_id = 102;
        strcpy(potion.name, "Mega Health Potion");
        potion.type = ITEM_POTION;
        potion.stat_value = 100;
        potion.price = 60;
        potion.quantity = 1;
    } else {
        potion.item_id = 101;
        strcpy(potion.name, "Small Health Potion");
        potion.type = ITEM_POTION;
        potion.stat_value = 40;
        potion.price = 25;
        potion.quantity = 1;
    }

    // Cek apakah item sudah ada di tas untuk ditumpuk
    for (int i = 0; i < p->item_count; i++) {
        if (p->inventory[i].item_id == potion.item_id) {
            p->inventory[i].quantity += 1;
            return 1;
        }
    }

    // Jika belum ada, masukkan ke slot kosong jika masih muat
    if (p->item_count < MAX_INVENTORY) {
        p->inventory[p->item_count] = potion;
        p->item_count++;
        return 1;
    }

    return 0;
}

// Eksplorasi area berdasarkan pilihan arah mata angin
void exploreAreaByDirection(struct Player *p, int area_type) {
    char nama_area[30];
    if (area_type == 2) {
        strcpy(nama_area, "Hutan Rimba");
    } else {
        strcpy(nama_area, "Gua Kegelapan");
    }

    char nama_arah[4][15] = {"Utara", "Selatan", "Timur", "Barat"};
    int langkah = 0;

    while (p->hp > 0) {
        clearScreen();
        printf("==================================================\n");
        printf("  LOKASI: %-15s | Langkah ke-%d\n", nama_area, langkah);
        printf("  %s (Lv.%d) | HP: %d/%d | Gold: %d G\n",
               p->name, p->level, p->hp, p->max_hp, p->gold);
        printf("==================================================\n");
        printf("Pilih arah langkahmu:\n");
        printf(" 1. Melangkah ke Utara\n");
        printf(" 2. Melangkah ke Selatan\n");
        printf(" 3. Melangkah ke Timur\n");
        printf(" 4. Melangkah ke Barat\n");
        printf(" 0. Keluar dari %s (Kembali ke Peta Dunia)\n", nama_area);
        printf("==================================================\n");
        printf("Pilihan arah (0-4): ");

        int arah = getValidIntInput(0, 4);
        if (arah == 0) {
            printf("\n>> Kamu berjalan keluar meninggalkan %s...\n", nama_area);
            pauseScreen();
            break;
        }

        langkah++;
        clearScreen();
        printf(">> Kamu melangkah menuju arah %s di dalam %s...\n\n",
               nama_arah[arah - 1], nama_area);

        int acak = rand() % 100;

        if (acak < 50) {
            // 50% peluang bertemu monster
            struct Monster musuh;
            if (area_type == 2) {
                musuh = spawnForestMonster();
            } else {
                musuh = spawnCaveMonster();
            }

            printf("!! WASPADA !! Dari arah %s muncul %s (Lv.%d)!\n",
                   nama_arah[arah - 1], musuh.name, musuh.level_req);
            pauseScreen();

            int hasil = startCombat(p, musuh, 1);
            if (hasil == COMBAT_LOSE) {
                return;
            }

        } else if (acak < 55) {
            // 5% peluang menemukan chest
            int acak_chest = rand() % 100;
            int dapat_gold = 0;
            if (area_type == 2) {
                dapat_gold = 15 + (rand() % 26);
            } else {
                dapat_gold = 40 + (rand() % 51);
            }
            p->gold += dapat_gold;

            printf("==================================================\n");
            printf("  [CHEST DITEMUKAN!] Kamu membuka peti harta karun!\n");
            printf("  >> Mendapatkan %d Gold! (Total: %d G)\n", dapat_gold, p->gold);

            if (acak_chest < 45) {
                int is_mega = 0;
                if (area_type == 3) {
                    is_mega = 1;
                }

                if (giveChestPotion(p, is_mega) == 1) {
                    if (is_mega == 1) {
                        printf("  >> Bonus item: 1x Mega Health Potion masuk ke tas!\n");
                    } else {
                        printf("  >> Bonus item: 1x Small Health Potion masuk ke tas!\n");
                    }
                } else {
                    printf("  >> Ada Potion di dalam peti, tapi tasmu penuh!\n");
                }
            }
            printf("==================================================\n");
            pauseScreen();

        } else if (acak < 75) {
            // 20% peluang menemukan mata air pemulih HP
            int jumlah_heal = 30;
            if (area_type == 2) {
                jumlah_heal = 15;
            }

            int hp_lama = p->hp;
            p->hp += jumlah_heal;
            if (p->hp > p->max_hp) {
                p->hp = p->max_hp;
            }

            printf("++ Kamu menemukan mata air jernih di balik bebatuan.\n");
            printf("   Kamu meminumnya dan HP pulih +%d (%d -> %d).\n",
                   p->hp - hp_lama, hp_lama, p->hp);
            pauseScreen();

        } else {
            // 25% peluang aman
            if (area_type == 2) {
                printf(".. Hanya suara angin dan dedaunan bergesekan. Jalur %s aman.\n",
                       nama_arah[arah - 1]);
            } else {
                printf(".. Lorong gua di arah %s gelap dan sunyi. Tidak ada apa-apa di sini.\n",
                       nama_arah[arah - 1]);
            }
            pauseScreen();
        }
    }
}

void restAtVillage(struct Player *p) {
    clearScreen();

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

    int pilihan = getValidIntInput(1, 2);
    if (pilihan == 1) {
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

// Buat boss gelombang ke-wave untuk mode Boss Rush
struct Monster spawnBossRushMonster(int wave) {
    char daftar_nama[BOSS_RUSH_NAMES_COUNT][30] = {
        "Malakor's Echo",
        "Crimson Revenant",
        "Ashen Warlord",
        "Void Herald",
        "Twin of the Abyss",
        "The Unnamed King"
    };

    struct Monster m;
    int idx = (wave - 1) % BOSS_RUSH_NAMES_COUNT;

    m.monster_id = 9000 + wave;
    strcpy(m.name, daftar_nama[idx]);
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

    int konfirmasi = getValidIntInput(1, 2);
    if (konfirmasi != 1) {
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

        int hasil = startCombat(p, boss, 0);

        if (hasil == COMBAT_LOSE) {
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
        if (p->boss_rush_unlocked == 1) {
            printf(" 5. BOSS RUSH       [MODE BARU]  - Gelombang boss tanpa akhir (mati = hapus)\n");
        }
        printf(" 0. Kembali ke Menu Utama\n");
        printf("==================================================\n");

        int batas_pilihan = 4;
        if (p->boss_rush_unlocked == 1) {
            batas_pilihan = 5;
            printf("Pilih tujuan jelajah (0-5): ");
        } else {
            printf("Pilih tujuan jelajah (0-4): ");
        }

        int pilihan = getValidIntInput(0, batas_pilihan);
        if (pilihan == 0) {
            break;
        }

        if (pilihan == 1) {
            restAtVillage(p);
        } else if (pilihan == 2 || pilihan == 3) {
            exploreAreaByDirection(p, pilihan);
        } else if (pilihan == 5 && p->boss_rush_unlocked == 1) {
            startBossRush(p);
        } else if (pilihan == 4) {
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

            int konfirmasi = getValidIntInput(1, 2);
            if (konfirmasi == 1) {
                struct Monster boss = spawnBossMonster();
                int hasil = startCombat(p, boss, 1);

                if (hasil == COMBAT_WIN) {
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
                } else if (hasil == COMBAT_LOSE) {
                    return;
                }
            }
        }
    }
}