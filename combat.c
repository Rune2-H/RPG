#include "game.h"

#define CRIT_CHANCE          20
#define MONSTER_CRIT_CHANCE  10
#define SKILL_COOLDOWN       3
#define SPECIAL_COOLDOWN     3
#define BUFF_TURNS           3
#define MAGE_OVERCHANNEL_PCT 10

// Hitung damage dasar dikurangi defense, kali 2 jika critical
int calculateDamage(int atk, int def, int is_critical) {
    int base_dmg = atk - def;
    if (base_dmg < 1) {
        base_dmg = 1;
    }
    if (is_critical == 1) {
        base_dmg = base_dmg * 2;
    }
    return base_dmg;
}

// Ambil nama dan deskripsi aksi khusus sesuai class player
void getClassSpecial(char class_type[], char nama_aksi[], char info_aksi[]) {
    if (strcmp(class_type, "Warrior") == 0) {
        strcpy(nama_aksi, "Berserk Cleave");
        strcpy(info_aksi, "crit pasti + ATK +50% (3 giliran), tapi damage diterima 2x");
    } else if (strcmp(class_type, "Mage") == 0) {
        strcpy(nama_aksi, "Overchannel");
        strcpy(info_aksi, "crit pasti menembus DEF, bayar 10% Max HP, damage diterima 2x");
    } else {
        strcpy(nama_aksi, "Twin Fangs");
        strcpy(info_aksi, "dua serangan crit pasti, tapi damage diterima 2x");
    }
}

int startCombat(struct Player *p, struct Monster m, int allow_flee) {
    int skill_cooldown = 0;
    int special_cooldown = 0;
    int atk_buff_turns = 0;

    char nama_special[30];
    char info_special[100];
    getClassSpecial(p->class_type, nama_special, info_special);

    clearScreen();
    printf("==========================================\n");
    printf("       PERTARUNGAN DIMULAI!               \n");
    printf("   %s (Lv.%d) menghadangmu!\n", m.name, m.level_req);
    printf("==========================================\n");
    pauseScreen();

    // Loop pertarungan selama player dan monster masih hidup
    while (p->hp > 0 && m.hp > 0) {
        int total_atk = p->base_atk + p->bonus_atk;
        int total_def = p->base_def + p->bonus_def;
        int guard = 0;
        int vulnerable = 0;

        if (atk_buff_turns > 0) {
            total_atk = total_atk * 3 / 2;
        }

        // Tampilkan status pertarungan
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

        if (allow_flee == 1) {
            printf("4. Kabur (Run)\n");
        } else {
            printf("4. Kabur [TERKUNCI - Boss Rush]\n");
        }

        printf("5. Full Guard [blokir total serangan musuh]\n");

        if (special_cooldown == 0) {
            printf("6. %s [SIAP]\n", nama_special);
        } else {
            printf("6. %s [Cooldown: %d giliran]\n", nama_special, special_cooldown);
        }
        printf("   %s: %s\n", nama_special, info_special);
        printf("Pilihan (1-6): ");

        int pilihan = getValidIntInput(1, 6);
        int giliran_terpakai = 0;

        printf("\n------------------------------------------\n");

        // --- GILIRAN PLAYER ---
        if (pilihan == 1) {
            int is_crit = 0;
            if ((rand() % 100) < CRIT_CHANCE) {
                is_crit = 1;
            }

            int dmg = calculateDamage(total_atk, m.def, is_crit);
            m.hp -= dmg;
            if (m.hp < 0) {
                m.hp = 0;
            }

            if (is_crit == 1) {
                printf(">> CRITICAL HIT! %s menebas %s sebesar %d damage!\n", p->name, m.name, dmg);
            } else {
                printf(">> %s menyerang %s sebesar %d damage!\n", p->name, m.name, dmg);
            }
            giliran_terpakai = 1;

        } else if (pilihan == 2) {
            if (skill_cooldown > 0) {
                printf(">> Skill masih cooldown (%d giliran lagi)! Pilih aksi lain.\n", skill_cooldown);
                pauseScreen();
                continue;
            }

            int dmg = 0;
            if (strcmp(p->class_type, "Warrior") == 0) {
                dmg = calculateDamage((int)(total_atk * 1.8), m.def, 0);
                printf(">> [HEAVY SLASH] %s menghantam keras %s sebesar %d damage!\n", p->name, m.name, dmg);
            } else if (strcmp(p->class_type, "Mage") == 0) {
                dmg = calculateDamage((int)(total_atk * 1.6), m.def / 2, 0);
                printf(">> [FIREBALL] Bola api membakar %s sebesar %d damage!\n", m.name, dmg);
            } else {
                dmg = calculateDamage(total_atk, m.def, 1);
                printf(">> [BACKSTAB] Serangan titik lemah %s menghasilkan %d CRITICAL damage!\n", m.name, dmg);
            }

            m.hp -= dmg;
            if (m.hp < 0) {
                m.hp = 0;
            }
            skill_cooldown = SKILL_COOLDOWN;
            giliran_terpakai = 1;

        } else if (pilihan == 3) {
            if (usePotionInCombat(p) == 1) {
                giliran_terpakai = 1;
            } else {
                pauseScreen();
                continue;
            }

        } else if (pilihan == 4) {
            if (allow_flee == 0) {
                printf(">> Kamu mencoba kabur, tapi gerbang arena Boss Rush terkunci rapat!\n");
                pauseScreen();
                continue;
            }

            if ((rand() % 100) < 50) {
                printf(">> Kamu berhasil melarikan diri dari %s!\n", m.name);
                pauseScreen();
                return COMBAT_RUN;
            } else {
                printf(">> Gagal kabur! %s menghadang jalanmu!\n", m.name);
                giliran_terpakai = 1;
            }

        } else if (pilihan == 5) {
            printf(">> %s mengangkat senjata dan mengunci pertahanan penuh...\n", p->name);
            guard = 1;
            giliran_terpakai = 1;

        } else if (pilihan == 6) {
            if (special_cooldown > 0) {
                printf(">> %s masih cooldown (%d giliran lagi)!\n", nama_special, special_cooldown);
                pauseScreen();
                continue;
            }

            special_cooldown = SPECIAL_COOLDOWN;

            if (strcmp(p->class_type, "Warrior") == 0) {
                int dmg = calculateDamage(total_atk * 3 / 2, m.def, 1);
                m.hp -= dmg;
                if (m.hp < 0) {
                    m.hp = 0;
                }

                printf(">> [BERSERK CLEAVE] %s menebas dengan seluruh tenaga: %d CRITICAL damage!\n", p->name, dmg);
                atk_buff_turns = BUFF_TURNS;
                printf(">> Nafsu bertarung naik: ATK +50%% selama %d giliran.\n", BUFF_TURNS);
                vulnerable = 1;
                printf(">> PERTAHANAN TERBUKA: kamu menerima damage 2x lipat giliran ini!\n");

            } else if (strcmp(p->class_type, "Mage") == 0) {
                int biaya_hp = p->max_hp * MAGE_OVERCHANNEL_PCT / 100;
                if (biaya_hp < 1) {
                    biaya_hp = 1;
                }

                p->hp -= biaya_hp;
                printf(">> [OVERCHANNEL] %s membakar %d HP untuk membuka aliran mana!\n", p->name, biaya_hp);
                if (p->hp < 1) {
                    p->hp = 1;
                    printf(">> Sisa hidupmu tinggal 1 HP!\n");
                }

                int dmg = calculateDamage(total_atk * 3 / 2, 0, 1);
                m.hp -= dmg;
                if (m.hp < 0) {
                    m.hp = 0;
                }

                printf(">> Sihir mentah menembus pertahanan %s: %d CRITICAL damage!\n", m.name, dmg);
                vulnerable = 1;
                printf(">> Mana tak terkendali: damage yang kamu terima 2x lipat giliran ini!\n");

            } else {
                int dmg1 = calculateDamage(total_atk * 3 / 4, m.def, 1);
                int dmg2 = calculateDamage(total_atk * 3 / 4, m.def / 2, 1);
                m.hp -= dmg1 + dmg2;
                if (m.hp < 0) {
                    m.hp = 0;
                }

                printf(">> [TWIN FANGS] Dua tusukan dari %s: %d + %d CRITICAL damage!\n", p->name, dmg1, dmg2);
                vulnerable = 1;
                printf(">> Kamu terlalu dekat: damage yang kamu terima 2x lipat giliran ini!\n");
            }

            giliran_terpakai = 1;
        }

        // Cek apakah monster sudah kalah
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
        if (giliran_terpakai == 1) {
            if (guard == 1) {
                printf("\n>> get parried nerd, 0 damage taken\n");
            } else {
                int monster_crit = 0;
                if ((rand() % 100) < MONSTER_CRIT_CHANCE) {
                    monster_crit = 1;
                }

                int monster_dmg = calculateDamage(m.atk, total_def, monster_crit);
                if (vulnerable == 1) {
                    monster_dmg = monster_dmg * 2;
                }

                p->hp -= monster_dmg;
                if (p->hp < 0) {
                    p->hp = 0;
                }

                if (monster_crit == 1) {
                    printf(">> SERANGAN TELAK! %s menyerang %s sebesar %d damage!\n", m.name, p->name, monster_dmg);
                } else {
                    printf(">> %s membalas serangan sebesar %d damage!\n", m.name, monster_dmg);
                }

                if (vulnerable == 1) {
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

    // Jika keluar dari loop karena HP player habis
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