#include "game.h"

// ============================================================================
// MODUL 2: combat.c (Sistem Pertarungan Turn-Based)
// ============================================================================

static int calculateDamage(int atk, int def, int is_critical) {
    int base_dmg = atk - def;
    if (base_dmg < 1) {
        base_dmg = 1; // Minimal selalu masuk 1 damage
    }
    if (is_critical) {
        base_dmg *= 2;
    }
    return base_dmg;
}

int startCombat(struct Player *p, struct Monster m) {
    // Variabel sesi pertarungan (otomatis reset setelah pertarungan selesai)
    int temp_atk_buff = 0;      // Tambahan ATK sementara (dari skill seperti Roar)
    int special_used = 0;       // 0 = belum dipakai, 1 = sudah dipakai (1x per sesi)
    int skill_cooldown = 0;     // Cooldown untuk class selain Warrior (Mage/Rogue)

    int is_warrior = (strcmp(p->class_type, "Warrior") == 0);

    clearScreen();
    printf("==========================================\n");
    printf("       PERTARUNGAN DIMULAI!               \n");
    printf("   %s (Lv.%d) menghadangmu!\n", m.name, m.level_req);
    printf("==========================================\n");
    pauseScreen();

    while (p->hp > 0 && m.hp > 0) {
        clearScreen();
        int total_atk = p->base_atk + p->bonus_atk + temp_atk_buff;
        int total_def = p->base_def + p->bonus_def;

        // Tampilkan HUD Pertarungan
        printf("==========================================\n");
        printf(" [MUSUH] %s (Lv.%d)\n", m.name, m.level_req);
        printf(" HP: %d / %d | ATK: %d | DEF: %d\n", m.hp, m.max_hp, m.atk, m.def);
        printf("------------------------------------------\n");
        printf(" [KAMU]  %s (%s - Lv.%d)\n", p->name, p->class_type, p->level);
        if (temp_atk_buff > 0) {
            printf(" HP: %d / %d | ATK: %d (+%d Buff) | DEF: %d\n",
                   p->hp, p->max_hp, total_atk, temp_atk_buff, total_def);
        } else {
            printf(" HP: %d / %d | ATK: %d | DEF: %d\n",
                   p->hp, p->max_hp, total_atk, total_def);
        }
        printf("==========================================\n");
        printf("Pilih Aksi:\n");

        int max_choice = 4;
        if (is_warrior) {
            max_choice = 5;
            printf("1. Slash (Serangan Biasa)\n");
            printf("2. Roar (+5 ATK selama pertarungan)\n");
            if (!special_used) {
                printf("3. Heavy Slash [SPESIAL - 1x Per Pertarungan]\n");
            } else {
                printf("3. Heavy Slash [SUDAH DIGUNAKAN]\n");
            }
            printf("4. Gunakan Potion\n");
            printf("5. Kabur (Run)\n");
            printf("Pilihan (1-5): ");
        } else {
            // Menu default sementara untuk Mage & Rogue
            printf("1. Serang Biasa (Attack)\n");
            if (skill_cooldown == 0) {
                printf("2. Skill Spesial %s [SIAP]\n", p->class_type);
            } else {
                printf("2. Skill Spesial %s [Cooldown: %d giliran]\n", p->class_type, skill_cooldown);
            }
            printf("3. Gunakan Potion\n");
            printf("4. Kabur (Run)\n");
            printf("Pilihan (1-4): ");
        }

        int choice = getValidIntInput(1, max_choice);
        int player_acted = 0;

        printf("\n------------------------------------------\n");

        // --- ALUR KHUSUS WARRIOR ---
        if (is_warrior) {
            if (choice == 1) {
                // Slash
                int is_crit = (rand() % 100) < 20;
                int dmg = calculateDamage(total_atk, m.def, is_crit);
                m.hp -= dmg;
                if (m.hp < 0) m.hp = 0;

                if (is_crit) {
                    printf(">> CRITICAL SLASH! %s menebas %s sebesar %d damage!\n", p->name, m.name, dmg);
                } else {
                    printf(">> [SLASH] %s menebas %s sebesar %d damage!\n", p->name, m.name, dmg);
                }
                player_acted = 1;

            } else if (choice == 2) {
                // Roar (+5 ATK sementara)
                temp_atk_buff += 5;
                printf(">> [ROAR] %s mengaum garang! ATK meningkat +5 (Total ATK sekarang: %d)!\n",
                       p->name, total_atk + 5);
                player_acted = 1;

            } else if (choice == 3) {
                // Heavy Slash (Spesial - 1x per sesi)
                if (special_used) {
                    printf(">> Heavy Slash hanya bisa digunakan 1 kali per pertarungan! Pilih aksi lain.\n");
                    pauseScreen();
                    continue;
                }

                int dmg = calculateDamage((int)(total_atk * 1.8), m.def, 0);
                m.hp -= dmg;
                if (m.hp < 0) m.hp = 0;

                printf(">> [HEAVY SLASH] Serangan pamungkas %s menghantam %s sebesar %d damage!\n",
                       p->name, m.name, dmg);
                special_used = 1;
                player_acted = 1;

            } else if (choice == 4) {
                if (usePotionInCombat(p)) {
                    player_acted = 1;
                } else {
                    pauseScreen();
                    continue;
                }

            } else if (choice == 5) {
                if ((rand() % 100) < 50) {
                    printf(">> Kamu berhasil melarikan diri dari %s!\n", m.name);
                    pauseScreen();
                    return COMBAT_RUN;
                } else {
                    printf(">> Gagal kabur! %s menghadang jalanmu!\n", m.name);
                    player_acted = 1;
                }
            }
        }
        // --- ALUR MAGE & ROGUE (SEMENTARA) ---
        else {
            if (choice == 1) {
                int is_crit = (rand() % 100) < 20;
                int dmg = calculateDamage(total_atk, m.def, is_crit);
                m.hp -= dmg;
                if (m.hp < 0) m.hp = 0;

                if (is_crit) {
                    printf(">> CRITICAL HIT! %s menyerang %s sebesar %d damage!\n", p->name, m.name, dmg);
                } else {
                    printf(">> %s menyerang %s sebesar %d damage!\n", p->name, m.name, dmg);
                }
                player_acted = 1;

            } else if (choice == 2) {
                if (skill_cooldown > 0) {
                    printf(">> Skill masih cooldown (%d giliran lagi)! Pilih aksi lain.\n", skill_cooldown);
                    pauseScreen();
                    continue;
                }

                int dmg = 0;
                if (strcmp(p->class_type, "Mage") == 0) {
                    dmg = calculateDamage((int)(total_atk * 1.6), m.def / 2, 0);
                    printf(">> [FIREBALL] Bola api membakar %s sebesar %d damage!\n", m.name, dmg);
                } else {
                    dmg = calculateDamage(total_atk, m.def, 1);
                    printf(">> [BACKSTAB] Serangan titik lemah %s menghasilkan %d CRITICAL damage!\n", m.name, dmg);
                }

                m.hp -= dmg;
                if (m.hp < 0) m.hp = 0;
                skill_cooldown = 3;
                player_acted = 1;

            } else if (choice == 3) {
                if (usePotionInCombat(p)) {
                    player_acted = 1;
                } else {
                    pauseScreen();
                    continue;
                }

            } else if (choice == 4) {
                if ((rand() % 100) < 50) {
                    printf(">> Kamu berhasil melarikan diri dari %s!\n", m.name);
                    pauseScreen();
                    return COMBAT_RUN;
                } else {
                    printf(">> Gagal kabur! %s menghadang jalanmu!\n", m.name);
                    player_acted = 1;
                }
            }
        }

        // Cek apakah monster kalah
        if (m.hp <= 0) {
            printf("\n==========================================\n");
            printf("  KEMENANGAN! %s telah dikalahkan!\n", m.name);
            printf("  Hadiah: +%d Gold | +%d EXP\n", m.gold_reward, m.exp_reward);
            printf("==========================================\n");

            p->gold += m.gold_reward;
            p->exp += m.exp_reward;

            checkLevelUp(p);
            pauseScreen();
            return COMBAT_WIN;
        }

        // --- GILIRAN MONSTER ---
        if (player_acted) {
            int monster_crit = (rand() % 100) < 10;
            int monster_dmg = calculateDamage(m.atk, total_def, monster_crit);
            p->hp -= monster_dmg;
            if (p->hp < 0) p->hp = 0;

            if (monster_crit) {
                printf(">> SERANGAN TELAK! %s menyerang %s sebesar %d damage!\n", m.name, p->name, monster_dmg);
            } else {
                printf(">> %s membalas serangan sebesar %d damage!\n", m.name, monster_dmg);
            }

            if (skill_cooldown > 0) {
                skill_cooldown--;
            }

            pauseScreen();
        }
    }

    if (p->hp <= 0) {
        clearScreen();
        printf("==========================================\n");
        printf("               GAME OVER                  \n");
        printf("   %s tumbang di tangan %s...\n", p->name, m.name);
        printf("==========================================\n");
        pauseScreen();
        return COMBAT_LOSE;
    }

    return COMBAT_WIN;
}