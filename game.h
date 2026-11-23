#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// 1. KONSTANTA & DEFINISI GLOBAL
// ============================================================================
#define MAX_NAME_LEN 50
#define MAX_INVENTORY 20
#define SAVE_FILE "savegame.txt"

// Kategori Item (Mempermudah ENUM / Lookup Table di Basis Data Semester 3)
#define ITEM_POTION 1
#define ITEM_WEAPON 2
#define ITEM_ARMOR  3

// Status Hasil Pertarungan (Return value untuk combat.c)
#define COMBAT_LOSE 0
#define COMBAT_WIN  1
#define COMBAT_RUN  2

// ============================================================================
// 2. STRUKTUR DATA (DIRANCANG RELASIONAL UNTUK MIGRASI KE SQL SEMESTER 3)
// ============================================================================

// Representasi Tabel: items / player_inventory
struct Item {
    int item_id;                  // Primary Key (contoh: 101 = Potion, 201 = Iron Sword)
    char name[MAX_NAME_LEN];      // Nama item
    int type;                     // 1: Potion, 2: Weapon, 3: Armor
    int stat_value;               // Jumlah heal HP (Potion), bonus ATK (Weapon), atau bonus DEF (Armor)
    int price;                    // Harga beli di toko
    int quantity;                 // Jumlah item yang dimiliki di tas
};

// Representasi Tabel: players
struct Player {
    int player_id;                // Primary Key
    char name[MAX_NAME_LEN];      // Nama karakter
    char class_type[20];          // "Warrior", "Mage", atau "Rogue"
    
    // Statistik Dasar & Progresi
    int level;
    int exp;
    int hp;
    int max_hp;
    int base_atk;
    int base_def;
    int gold;

    // Relasi Equipment (Foreign Key mengacu ke item_id)
    int equipped_weapon_id;       // 0 jika tidak memakai senjata
    int equipped_armor_id;        // 0 jika tidak memakai armor
    int bonus_atk;                // Tambahan ATK dari senjata yang sedang dipakai
    int bonus_def;                // Tambahan DEF dari armor yang sedang dipakai

    // Relasi Tas (1-to-Many)
    struct Item inventory[MAX_INVENTORY];
    int item_count;               // Jumlah jenis item unik yang sedang ada di tas

    // Progresi Mode Tambahan
    int boss_rush_unlocked;       // 1 jika Raja Iblis sudah pernah dikalahkan
};

// Representasi Tabel: monsters
struct Monster {
    int monster_id;               // Primary Key
    char name[MAX_NAME_LEN];      // Nama monster
    int level_req;                // Tingkat kesulitan / level monster
    int hp;
    int max_hp;
    int atk;
    int def;
    int gold_reward;
    int exp_reward;
};

// ============================================================================
// 3. FUNCTION PROTOTYPES (KONTRAK ANTAR MODUL)
// ============================================================================

// --- MODUL 1: player.c (Anggota 1) ---
void createCharacter(struct Player *p);
void showStatus(struct Player p);
void checkLevelUp(struct Player *p);

// --- MODUL 2: combat.c (Anggota 2) ---
// allow_flee: 1 = tombol kabur aktif, 0 = terkunci (dipakai Boss Rush)
int startCombat(struct Player *p, struct Monster m, int allow_flee);

// --- MODUL 3: inventory.c (Anggota 3) ---
void openShop(struct Player *p);
void openInventory(struct Player *p);
int usePotionInCombat(struct Player *p); // Opsional: dipanggil saat player pilih pakai potion di combat.c

// --- MODUL 4: world.c (Anggota 4) ---
void exploreWorld(struct Player *p);
void restAtVillage(struct Player *p);
void startBossRush(struct Player *p);     // Mode tanpa akhir; mati = save dihapus

// --- MODUL 5: saveload.c (Anggota 5) ---
int saveGame(struct Player p);            // Return 1 jika berhasil, 0 jika gagal
int loadGame(struct Player *p);           // Return 1 jika file ada & berhasil dimuat, 0 jika gagal

// --- MODUL 6: main.c (Anggota 6 - Helper UI & Input) ---
void clearScreen();
void pauseScreen();
int getValidIntInput(int min, int max);

#endif // GAME_H