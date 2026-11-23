#include "game.h"

// ============================================================================
// MODUL 2: combat.c (Sistem Pertarungan Turn-Based)
// ============================================================================

// Fungsi pembantu internal (static agar tidak bentrok dengan file lain)
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
    int skill_cooldown = 0; // Cooldown skill spesial (0 = siap dipakai)

    clearScreen();
    printf("==========================================\n");
    printf("       PERTARUNGAN DIMULAI!               \n");
    printf("   %s (Lv.%d) menghadangmu!\n", m.name, m.level_req);
    printf("==========================================\n");
    pauseScreen();

    // Loop pertarungan selama Player dan Monster masih hidup
    while (p->hp > 0 && m.hp > 0) {
        clearScreen();
        int total_atk = p->base_atk + p->bonus_atk;
        int total_def = p->base_def + p->bonus_def;

        // Tampilkan HUD Pertarungan
        printf("==========================================\n");
        printf(" [MUSUH] %s (Lv.%d)\n", m.name, m.level_req);
        printf(" HP: %d / %d | ATK: %d | DEF: %d\n", m.hp, m.max_hp, m.atk, m.def);
        printf("------------------------------------------\n");
        printf(" [KAMU]  %s (%s - Lv.%d)\n", p->name, p->class_type, p->level);
        printf(" HP: %d / %d | ATK: %d | DEF: %d\n", p->hp, p->max_hp, total_atk, total_def);
        printf("==========================================\n");
        printf("Pilih Aksi:\n");
        printf("1. Serang Biasa (Attack)\n");
        if (skill_cooldown == 0) {
            printf("2. Skill Spesial %s [SIAP]\n", p->class_type);
        } else {
            printf("2. Skill Spesial %s [Cooldown: %d giliran]\n", p->class_type, skill_cooldown);
        }
        printf("3. Gunakan Potion\n");
        printf("4. Kabur (Run)\n");
        printf("Pilihan (1-4): ");

        int choice = getValidIntInput(1, 4);
        int player_acted = 0; // Penanda apakah giliran player sudah terpakai

        printf("\n------------------------------------------\n");

        // --- GILIRAN PLAYER ---
        if (choice == 1) {
            // Peluang Critical Hit 20%
            int is_crit = (rand() % 100) < 20;
            int dmg = calculateDamage(total_atk, m.def, is_crit);
            m.hp -= dmg;
            if (m.hp < 0) m.hp = 0;

            if (is_crit) {
                printf(">> CRITICAL HIT! %s menebas %s sebesar %d damage!\n", p->name, m.name, dmg);
            } else {
                printf(">> %s menyerang %s sebesar %d damage!\n", p->name, m.name, dmg);
            }
            player_acted = 1;

        } else if (choice == 2) {
            if (skill_cooldown > 0) {
                printf(">> Skill masih cooldown (%d giliran lagi)! Pilih aksi lain.\n", skill_cooldown);
                pauseScreen();
                continue; // Ulangi input tanpa membuang giliran
            }

            int dmg = 0;
            if (strcmp(p->class_type, "Warrior") == 0) {
                // Heavy Slash: 1.8x ATK
                dmg = calculateDamage((int)(total_atk * 1.8), m.def, 0);
                printf(">> [HEAVY SLASH] %s menghantam keras %s sebesar %d damage!\n", p->name, m.name, dmg);
            } else if (strcmp(p->class_type, "Mage") == 0) {
                // Fireball: Menembus setengah DEF musuh, 1.6x ATK
                dmg = calculateDamage((int)(total_atk * 1.6), m.def / 2, 0);
                printf(">> [FIREBALL] Bola api membakar %s sebesar %d damage!\n", m.name, dmg);
            } else {
                // Backstab (Rogue): Pasti Critical Hit!
                dmg = calculateDamage(total_atk, m.def, 1);
                printf(">> [BACKSTAB] Serangan titik lemah %s menghasilkan %d CRITICAL damage!\n", m.name, dmg);
            }

            m.hp -= dmg;
            if (m.hp < 0) m.hp = 0;
            skill_cooldown = 3; // Cooldown 3 giliran
            player_acted = 1;

        } else if (choice == 3) {
            // Memanggil fungsi dari inventory.c
            if (usePotionInCombat(p)) {
                player_acted = 1;
            } else {
                // Jika tidak punya potion atau HP sudah penuh, jangan buang giliran
                pauseScreen();
                continue;
            }

        } else if (choice == 4) {
            // Peluang kabur 50%
            if ((rand() % 100) < 50) {
                printf(">> Kamu berhasil melarikan diri dari %s!\n", m.name);
                pauseScreen();
                return COMBAT_RUN;
            } else {
                printf(">> Gagal kabur! %s menghadang jalanmu!\n", m.name);
                player_acted = 1;
            }
        }

        // Cek apakah monster sudah kalah setelah diserang player
        if (m.hp <= 0) {
            printf("\n==========================================\n");
            printf("  KEMENANGAN! %s telah dikalahkan!\n", m.name);
            printf("  Hadiah: +%d Gold | +%d EXP\n", m.gold_reward, m.exp_reward);
            printf("==========================================\n");

            p->gold += m.gold_reward;
            p->exp += m.exp_reward;

            // Panggil fungsi dari player.c untuk cek naik level
            checkLevelUp(p);
            pauseScreen();
            return COMBAT_WIN;
        }

        // --- GILIRAN MONSTER ---
        if (player_acted) {
            int monster_crit = (rand() % 100) < 10; // Peluang crit monster 10%
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

    // Jika keluar dari loop karena HP Player habis
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