#include "game.h"

// ============================================================================
// MODUL 2: combat.c (Sistem Pertarungan Turn-Based)
// ============================================================================

#define CRIT_CHANCE          20  // Peluang critical hit player (%)
#define MONSTER_CRIT_CHANCE  10  // Peluang critical hit monster (%)
#define SKILL_COOLDOWN       3   // Jeda skill class (dalam giliran)
#define SPECIAL_COOLDOWN     3   // Jeda aksi khusus class (dalam giliran)
#define BUFF_TURNS           3   // Lama ATK buff Warrior (dalam giliran)
#define MAGE_OVERCHANNEL_PCT 10  // Biaya HP Overchannel (% dari Max HP)

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

// --- AKSI KHUSUS PER CLASS (menu pilihan 6) ---

static const char *classSpecialName(const struct Player *p) {
    if (strcmp(p->class_type, "Warrior") == 0) {
        return "Berserk Cleave";
    }
    if (strcmp(p->class_type, "Mage") == 0) {
        return "Overchannel";
    }
    return "Twin Fangs";
}

static const char *classSpecialInfo(const struct Player *p) {
    if (strcmp(p->class_type, "Warrior") == 0) {
        return "crit pasti + ATK +50% (3 giliran), tapi damage diterima 2x";
    }
    if (strcmp(p->class_type, "Mage") == 0) {
        return "crit pasti menembus DEF, bayar 10% Max HP, damage diterima 2x";
    }
    return "dua serangan crit pasti, tapi damage diterima 2x";
}

int startCombat(struct Player *p, struct Monster m, int allow_flee) {
    int skill_cooldown = 0;    // Cooldown skill spesial (0 = siap dipakai)
    int special_cooldown = 0;  // Cooldown aksi khusus (0 = siap dipakai)
    int atk_buff_turns = 0;    // Sisa giliran ATK buff (Warrior)

    const char *special_name = classSpecialName(p);

    clearScreen();
    printf("==========================================\n");
    printf("       PERTARUNGAN DIMULAI!               \n");
    printf("   %s (Lv.%d) menghadangmu!\n", m.name, m.level_req);
    printf("==========================================\n");
    pauseScreen();

    // Loop pertarungan selama Player dan Monster masih hidup
    while (p->hp > 0 && m.hp > 0) {
        int total_atk = p->base_atk + p->bonus_atk;
        int total_def = p->base_def + p->bonus_def;
        int guard = 0;       // 1 = Full Guard aktif giliran ini
        int vulnerable = 0;  // 1 = damage yang diterima 2x giliran ini

        if (atk_buff_turns > 0) {
            total_atk = total_atk * 3 / 2; // Buff: +50% ATK
        }

        // Tampilkan HUD Pertarungan
        clearScreen();
        printf("==========================================\n");
        printf(" [MUSUH] %s (Lv.%d)\n", m.name, m.level_req);
        printf(" HP: %d / %d | ATK: %d | DEF: %d\n", m.hp, m.max_hp, m.atk, m.def);
        printf("------------------------------------------\n");
        printf(" [KAMU]  %s (%s - Lv.%d)\n", p->name, p->class_type, p->level);
        printf(" HP: %d / %d | ATK: %d | DEF: %d\n", p->hp, p->max_hp, total_atk, total_def);
        if (atk_buff_turns > 0) {
            printf(" STATUS  : ATK +50%% (%d giliran lagi)\n", atk_buff_turns);
        }
        printf("==========================================\n");
        printf("Pilih Aksi:\n");
        printf("1. Serang Biasa (Attack)\n");
        if (skill_cooldown == 0) {
            printf("2. Skill Spesial %s [SIAP]\n", p->class_type);
        } else {
            printf("2. Skill Spesial %s [Cooldown: %d giliran]\n", p->class_type, skill_cooldown);
        }
        printf("3. Gunakan Potion\n");
        if (allow_flee) {
            printf("4. Kabur (Run)\n");
        } else {
            printf("4. Kabur [TERKUNCI - Boss Rush]\n");
        }
        printf("5. Full Guard [blokir total serangan musuh]\n");
        if (special_cooldown == 0) {
            printf("6. %s [SIAP]\n", special_name);
        } else {
            printf("6. %s [Cooldown: %d giliran]\n", special_name, special_cooldown);
        }
        printf("   %s: %s\n", special_name, classSpecialInfo(p));
        printf("Pilihan (1-6): ");

        int choice = getValidIntInput(1, 6);
        int player_acted = 0; // Penanda apakah giliran player sudah terpakai

        printf("\n------------------------------------------\n");

        // --- GILIRAN PLAYER ---
        if (choice == 1) {
            // Peluang Critical Hit
            int is_crit = (rand() % 100) < CRIT_CHANCE;
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
            skill_cooldown = SKILL_COOLDOWN;
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
            if (!allow_flee) {
                printf(">> Kamu mencoba kabur, tapi gerbang arena Boss Rush terkunci rapat!\n");
                pauseScreen();
                continue; // Tidak membuang giliran, tapi juga tidak bisa keluar
            }

            // Peluang kabur 50%
            if ((rand() % 100) < 50) {
                printf(">> Kamu berhasil melarikan diri dari %s!\n", m.name);
                pauseScreen();
                return COMBAT_RUN;
            } else {
                printf(">> Gagal kabur! %s menghadang jalanmu!\n", m.name);
                player_acted = 1;
            }

        } else if (choice == 5) {
            // FULL GUARD: tidak menyerang, tapi serangan musuh giliran ini diblokir total
            printf(">> %s mengangkat senjata dan mengunci pertahanan penuh...\n", p->name);
            guard = 1;
            player_acted = 1;

        } else if (choice == 6) {
            if (special_cooldown > 0) {
                printf(">> %s masih cooldown (%d giliran lagi)!\n", special_name, special_cooldown);
                pauseScreen();
                continue; // Ulangi input tanpa membuang giliran
            }

            special_cooldown = SPECIAL_COOLDOWN;

            if (strcmp(p->class_type, "Warrior") == 0) {
                // Berserk Cleave: crit pasti + buff ATK, tapi pertahanan terbuka
                int dmg = calculateDamage(total_atk * 3 / 2, m.def, 1);
                m.hp -= dmg;
                if (m.hp < 0) m.hp = 0;

                printf(">> [BERSERK CLEAVE] %s menebas dengan seluruh tenaga: %d CRITICAL damage!\n", p->name, dmg);
                atk_buff_turns = BUFF_TURNS;
                printf(">> Nafsu bertarung naik: ATK +50%% selama %d giliran.\n", BUFF_TURNS);
                vulnerable = 1;
                printf(">> PERTAHANAN TERBUKA: kamu menerima damage 2x lipat giliran ini!\n");

            } else if (strcmp(p->class_type, "Mage") == 0) {
                // Overchannel: crit pasti yang menembus DEF, dibayar dengan HP
                int cost = p->max_hp * MAGE_OVERCHANNEL_PCT / 100;
                if (cost < 1) cost = 1;

                p->hp -= cost;
                printf(">> [OVERCHANNEL] %s membakar %d HP untuk membuka aliran mana!\n", p->name, cost);
                if (p->hp < 1) {
                    p->hp = 1; // Overchannel tidak boleh membunuh pemakainya sendiri
                    printf(">> Sisa hidupmu tinggal 1 HP!\n");
                }

                int dmg = calculateDamage(total_atk * 3 / 2, 0, 1); // DEF musuh diabaikan total
                m.hp -= dmg;
                if (m.hp < 0) m.hp = 0;

                printf(">> Sihir mentah menembus pertahanan %s: %d CRITICAL damage!\n", m.name, dmg);
                vulnerable = 1;
                printf(">> Mana tak terkendali: damage yang kamu terima 2x lipat giliran ini!\n");

            } else {
                // Twin Fangs: dua tusukan crit pasti
                int dmg1 = calculateDamage(total_atk * 3 / 4, m.def, 1);
                int dmg2 = calculateDamage(total_atk * 3 / 4, m.def / 2, 1);
                m.hp -= dmg1 + dmg2;
                if (m.hp < 0) m.hp = 0;

                printf(">> [TWIN FANGS] Dua tusukan dari %s: %d + %d CRITICAL damage!\n", p->name, dmg1, dmg2);
                vulnerable = 1;
                printf(">> Kamu terlalu dekat: damage yang kamu terima 2x lipat giliran ini!\n");
            }

            player_acted = 1;
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
            if (guard) {
                // Full Guard menelan seluruh serangan musuh
                printf("\n>> get parried nerd, 0 damage taken\n");
            } else {
                int monster_crit = (rand() % 100) < MONSTER_CRIT_CHANCE;
                int monster_dmg = calculateDamage(m.atk, total_def, monster_crit);

                if (vulnerable) {
                    monster_dmg *= 2;
                }

                p->hp -= monster_dmg;
                if (p->hp < 0) p->hp = 0;

                if (monster_crit) {
                    printf(">> SERANGAN TELAK! %s menyerang %s sebesar %d damage!\n", m.name, p->name, monster_dmg);
                } else {
                    printf(">> %s membalas serangan sebesar %d damage!\n", m.name, monster_dmg);
                }

                if (vulnerable) {
                    printf(">> (Damage diterima 2x lipat karena aksi khusus tadi!)\n");
                }
            }

            if (skill_cooldown > 0) {
                skill_cooldown--;
            }
            if (special_cooldown > 0) {
                special_cooldown--;
            }
            if (atk_buff_turns > 0) {
                atk_buff_turns--;
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
