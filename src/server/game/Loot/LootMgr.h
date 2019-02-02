/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINITY_LOOTMGR_H
#define TRINITY_LOOTMGR_H

#include "ItemEnchantmentMgr.h"
#include "ByteBuffer.h"
#include "RefManager.h"
#include "SharedDefines.h"
#include "ConditionMgr.h"
#include "Object.h"
#include "ItemPrototype.h"

#include <map>
#include <vector>

enum RollType
{
    ROLL_PASS         = 0,
    ROLL_NEED         = 1,
    ROLL_GREED        = 2,
    ROLL_DISENCHANT   = 3,
    MAX_ROLL_TYPE     = 4
};

enum RollMask
{
    ROLL_FLAG_TYPE_PASS         = 0x01,
    ROLL_FLAG_TYPE_NEED         = 0x02,
    ROLL_FLAG_TYPE_GREED        = 0x04,
    ROLL_FLAG_TYPE_DISENCHANT   = 0x08,

    ROLL_ALL_TYPE_NO_DISENCHANT = 0x07,
    ROLL_ALL_TYPE_MASK          = 0x0F
};

#define MAX_NR_LOOT_ITEMS 16
// note: the client cannot show more than 16 items total
#define MAX_NR_QUEST_ITEMS 32
// unrelated to the number of quest items shown, just for reserve

enum LootMethod
{
    FREE_FOR_ALL      = 0,
    ROUND_ROBIN       = 1,
    MASTER_LOOT       = 2,
    GROUP_LOOT        = 3,
    NEED_BEFORE_GREED = 4
};

enum PermissionTypes
{
    ALL_PERMISSION              = 0,
    GROUP_PERMISSION            = 1,
    MASTER_PERMISSION           = 2,
    ROUND_ROBIN_PERMISSION      = 3,
    OWNER_PERMISSION            = 4,
    NONE_PERMISSION             = 5,
};

enum LootItemType
{
    LOOT_ITEM_TYPE_ITEM     = 0,
    LOOT_ITEM_TYPE_CURRENCY = 1,
};

enum LootType
{
    LOOT_CORPSE                 = 1,
    LOOT_PICKPOCKETING          = 2,
    LOOT_FISHING                = 3,
    LOOT_DISENCHANTING          = 4,
                                                            // ignored always by client
    LOOT_SKINNING               = 6,
    LOOT_PROSPECTING            = 7,
    LOOT_MILLING                = 8,

    LOOT_FISHINGHOLE            = 20,                       // unsupported by client, sending LOOT_FISHING instead
    LOOT_INSIGNIA               = 21                        // unsupported by client, sending LOOT_CORPSE instead
};

// type of Loot Item in Loot View
enum LootSlotType
{
    LOOT_SLOT_TYPE_ROLL_ONGOING = 0,                        // roll is ongoing. player cannot loot.
    LOOT_SLOT_TYPE_LOCKED       = 1,                        // item is shown in red. player cannot loot.
    LOOT_SLOT_TYPE_MASTER       = 2,                        // item can only be distributed by group loot master.
    LOOT_SLOT_TYPE_ALLOW_LOOT   = 5,                        // player can loot the item.
    LOOT_SLOT_TYPE_OWNER        = 6                         // ignore binding confirmation and etc, for single player looting
};

class Player;
class LootStore;
class ConditionMgr;

struct LootStoreItem
{
    uint32  itemid;                                         // id of the item
    uint8   type;                                           // 0 = item, 1 = currency
    float   chance;                                         // always positive, chance to drop for both quest and non-quest items, chance to be used for refs
    int32   mincountOrRef;                                  // mincount for drop items (positive) or minus referenced TemplateleId (negative)
    uint16  lootmode;
    uint8   group       :7;
    bool    needs_quest :1;                                 // quest drop (negative ChanceOrQuestChance in DB)
    uint32   maxcount;                                      // max drop count for the item (mincountOrRef positive) or Ref multiplicator (mincountOrRef negative)
    ConditionContainer conditions;                               // additional loot condition

    // Constructor, converting ChanceOrQuestChance -> (chance, needs_quest)
    // displayid is filled in IsValid() which must be called after
    LootStoreItem(uint32 _itemid, uint8 _type, float _chanceOrQuestChance, uint16 _lootmode, uint8 _group, int32 _mincountOrRef, uint32 _maxcount)
        : itemid(_itemid), type(_type), chance(fabs(_chanceOrQuestChance)), mincountOrRef(_mincountOrRef), lootmode(_lootmode),
        group(_group), needs_quest(_chanceOrQuestChance < 0), maxcount(_maxcount)
         {}

    bool Roll(bool rate) const;                             // Checks if the entry takes it's chance (at loot generation)
    bool IsValid(LootStore const& store, uint32 entry) const;
                                                            // Checks correctness of values
};

typedef std::set<uint32> AllowedLooterSet;

struct Loot;

struct LootItem
{
    Loot* currentLoot;
    uint32  itemid;
    uint8   type;                                           // 0 = item, 1 = currency
    uint32  randomSuffix;
    int32   randomPropertyId;
    ConditionContainer conditions;                       // additional loot condition
    AllowedLooterSet allowedGUIDs;
    uint8   count             : 8;
    bool    currency          : 1;
    bool    is_looted         : 1;
    bool    is_blocked        : 1;
    bool    freeforall        : 1;                          // free for all
    bool    is_underthreshold : 1;
    bool    is_counted        : 1;
    bool    needs_quest       : 1;                          // quest drop
    bool    follow_loot_rules : 1;

    // Constructor, copies most fields from LootStoreItem, generates random count and random suffixes/properties
    // Should be called for non-reference LootStoreItem entries only (mincountOrRef > 0)
    explicit LootItem(LootStoreItem const& li, Loot* loot);

    // Basic checks for player/item compatibility - if false no chance to see the item in the loot
    bool AllowedForPlayer(Player const* player) const;

    void AddAllowedLooter(Player const* player);
    const AllowedLooterSet & GetAllowedLooters() const { return allowedGUIDs; }

    // Write packet data
    void WriteBitDataPart(uint8 slotType, ByteBuffer* buff);
    void WriteBasicDataPart(uint8 slot, ByteBuffer* buff);
};

struct QuestItem
{
    uint8   index;                                          // position in quest_items;
    bool    is_looted;

    QuestItem()
        : index(0), is_looted(false) {}

    QuestItem(uint8 _index, bool _islooted = false)
        : index(_index), is_looted(_islooted) {}
};

class LootTemplate;

typedef std::vector<QuestItem> QuestItemList;
typedef std::vector<LootItem> LootItemList;
typedef std::map<uint32, QuestItemList*> QuestItemMap;
typedef std::vector<LootStoreItem> LootStoreItemList;
typedef UNORDERED_MAP<uint32, LootTemplate*> LootTemplateMap;

typedef std::set<uint32> LootIdSet;

class LootStore
{
    public:
        explicit LootStore(char const* name, char const* entryName, bool ratesAllowed)
            : m_name(name), m_entryName(entryName), m_ratesAllowed(ratesAllowed) {}

        virtual ~LootStore() { Clear(); }

        void Verify() const;

        uint32 LoadAndCollectLootIds(LootIdSet& ids_set);
        void CheckLootRefs(LootIdSet* ref_set = NULL) const; // check existence reference and remove it from ref_set
        void ReportUnusedIds(LootIdSet const& ids_set) const;
        void ReportNotExistedId(uint32 id) const;

        bool HaveLootFor(uint32 loot_id) const { return m_LootTemplates.find(loot_id) != m_LootTemplates.end(); }
        bool HaveQuestLootFor(uint32 loot_id) const;
        bool HaveQuestLootForPlayer(uint32 loot_id, Player* player) const;

        LootTemplate const* GetLootFor(uint32 loot_id) const;
        void ResetConditions();
        LootTemplate* GetLootForConditionFill(uint32 loot_id);

        char const* GetName() const { return m_name; }
        char const* GetEntryName() const { return m_entryName; }
        bool IsRatesAllowed() const { return m_ratesAllowed; }
    protected:
        uint32 LoadLootTable();
        void Clear();
    private:
        LootTemplateMap m_LootTemplates;
        char const* m_name;
        char const* m_entryName;
        bool m_ratesAllowed;
};

class LootTemplate
{
    class LootGroup;                                       // A set of loot definitions for items (refs are not allowed inside)
    typedef std::vector<LootGroup> LootGroups;

    public:
        // Adds an entry to the group (at loading stage)
        void AddEntry(LootStoreItem& item);
        // Rolls for every item in the template and adds the rolled items the the loot
        void Process(Loot& loot, bool rate, uint16 lootMode, uint8 groupId = 0) const;
        void CopyConditions(ConditionContainer conditions);
        void FillAutoAssignationLoot(std::list<const ItemTemplate*>& p_ItemList) const;
        void FillAutoAssignationLootCurrency(std::list<std::pair<uint32/*currency*/, uint32/*count*/> >& currencies) const;

        // True if template includes at least 1 quest drop entry
        bool HasQuestDrop(LootTemplateMap const& store, uint8 groupId = 0) const;
        // True if template includes at least 1 quest drop for an active quest of the player
        bool HasQuestDropForPlayer(LootTemplateMap const& store, Player const* player, uint8 groupId = 0) const;

        // Checks integrity of the template
        void Verify(LootStore const& store, uint32 Id) const;
        void CheckLootRefs(LootTemplateMap const& store, LootIdSet* ref_set) const;
        bool addConditionItem(Condition* cond);
        bool isReference(uint32 id);

    private:
        LootStoreItemList Entries;                          // not grouped only
        LootGroups        Groups;                           // groups have own (optimised) processing, grouped entries go there
};

//=====================================================

class LootValidatorRef :  public Reference<Loot, LootValidatorRef>
{
    public:
        LootValidatorRef() {}
        void targetObjectDestroyLink() {}
        void sourceObjectDestroyLink() {}
};

//=====================================================

class LootValidatorRefManager : public RefManager<Loot, LootValidatorRef>
{
    public:
        typedef LinkedListHead::Iterator< LootValidatorRef > iterator;

        LootValidatorRef* getFirst() { return (LootValidatorRef*)RefManager<Loot, LootValidatorRef>::getFirst(); }
        LootValidatorRef* getLast() { return (LootValidatorRef*)RefManager<Loot, LootValidatorRef>::getLast(); }

        iterator begin() { return iterator(getFirst()); }
        iterator end() { return iterator(NULL); }
        iterator rbegin() { return iterator(getLast()); }
        iterator rend() { return iterator(NULL); }
};

//=====================================================
struct LootView;

struct LinkedLootInfo
{
    uint64 creatureGUID;
    uint32 slot;
    PermissionTypes permission;
};

struct StackableLinkedLootInfo
{
    uint64 creatureGUID;
    uint32 slot;
    uint8 count;
};

struct InstanceLooters
{
    public:

        InstanceLooters() : m_isEnabled(false) {}

        void SetEnabled(bool value) { m_isEnabled = value; }
        bool IsEnabled() const { return m_isEnabled; }

        void ClearGuids() { playerGuids.clear(); }
        void AddPlayerGuid(uint64 guid) { playerGuids.insert(guid); }
        bool HasPlayerGuid(uint64 guid) const { return (playerGuids.find(guid) != playerGuids.end()); }

    private:

        bool m_isEnabled;
        std::set<uint64> playerGuids;
};

struct Loot
{
    QuestItemMap const& GetPlayerCurrencies() const { return PlayerCurrencies; }
    QuestItemMap const& GetPlayerQuestItems() const { return PlayerQuestItems; }
    QuestItemMap const& GetPlayerFFAItems() const { return PlayerFFAItems; }
    QuestItemMap const& GetPlayerNonQuestNonFFANonCurrencyConditionalItems() const { return PlayerNonQuestNonFFANonCurrencyConditionalItems; }

    bool alreadyAskedForRoll;

    uint32 maxLinkedSlot;
    uint32 additionalLinkedGold;
    std::map<uint32, LinkedLootInfo> linkedLoot;
    std::map < uint32/*slot*/, StackableLinkedLootInfo> stackableLinkedLoot;
    std::vector<LootItem> items;
    std::vector<LootItem> quest_items;
    uint32 gold;
    uint8 unlootedCount;
    uint64 roundRobinPlayer;                                // GUID of the player having the Round-Robin ownership for the loot. If 0, round robin owner has released.
    LootType loot_type;                                     // required for achievement system

    Loot(uint32 _gold = 0) : gold(_gold), unlootedCount(0), loot_type(LOOT_CORPSE), alreadyAskedForRoll(false), maxLinkedSlot(0), additionalLinkedGold(0) {}
    ~Loot() { clear(); }

    // if loot becomes invalid this reference is used to inform the listener
    void addLootValidatorRef(LootValidatorRef* pLootValidatorRef)
    {
        i_LootValidatorRefManager.insertFirst(pLootValidatorRef);
    }

    // void clear();
    void clear()
    {
        for (QuestItemMap::const_iterator itr = PlayerCurrencies.begin(); itr != PlayerCurrencies.end(); ++itr)
            delete itr->second;
        PlayerCurrencies.clear();

        for (QuestItemMap::const_iterator itr = PlayerQuestItems.begin(); itr != PlayerQuestItems.end(); ++itr)
            delete itr->second;
        PlayerQuestItems.clear();

        for (QuestItemMap::const_iterator itr = PlayerFFAItems.begin(); itr != PlayerFFAItems.end(); ++itr)
            delete itr->second;
        PlayerFFAItems.clear();

        for (QuestItemMap::const_iterator itr = PlayerNonQuestNonFFANonCurrencyConditionalItems.begin(); itr != PlayerNonQuestNonFFANonCurrencyConditionalItems.end(); ++itr)
            delete itr->second;
        PlayerNonQuestNonFFANonCurrencyConditionalItems.clear();

        PlayersLooting.clear();
        items.clear();
        quest_items.clear();
        gold = 0;
        unlootedCount = 0;
        roundRobinPlayer = 0;
        additionalLinkedGold = 0;
        i_LootValidatorRefManager.clearReferences();
    }

    void addLinkedLoot(uint32 slot, uint64 linkedCreature, uint32 linkedSlot, PermissionTypes perm)
    {
        linkedLoot[slot].creatureGUID = linkedCreature;
        linkedLoot[slot].slot = linkedSlot;
        linkedLoot[slot].permission = perm;

        if (maxLinkedSlot < slot)
            maxLinkedSlot = slot;
    }

    bool isLinkedLoot(uint32 slot)
    {
        if (linkedLoot.find(slot) != linkedLoot.end())
            return true;

        return false;
    }

    // Must used only AFTER isLinkedLoot check
    LinkedLootInfo& getLinkedLoot(uint32 slot)
    {
        return linkedLoot[slot];
    }

    bool empty() const { return items.empty() && gold == 0; }
    bool isLooted() const { return gold == 0 && unlootedCount == 0; }

    void NotifyItemRemoved(uint8 lootIndex);
    void NotifyQuestItemRemoved(uint8 questIndex);
    void NotifyMoneyRemoved(uint64);
    void AddLooter(uint64 GUID) { PlayersLooting.insert(GUID); }
    void RemoveLooter(uint64 GUID) { PlayersLooting.erase(GUID); } 
    bool HasLooter(uint64 GUID) { return PlayersLooting.find(GUID) != PlayersLooting.end(); }

    void generateMoneyLoot(uint32 minAmount, uint32 maxAmount);
    bool FillLoot(uint32 lootId, LootStore const& store, Player* lootOwner, bool personal, bool noEmptyError = false, uint16 lootMode = LOOT_MODE_DEFAULT);

    // Inserts the item into the loot (called by LootTemplate processors)
    void AddItem(LootStoreItem const & item);

    LootItem* LootItemInSlot(uint32 lootslot, Player* player, QuestItem** qitem = NULL, QuestItem** ffaitem = NULL, QuestItem** conditem = NULL, QuestItem** currency = NULL);
    uint32 GetMaxSlotInLootFor(Player* player) const;
    bool hasItemFor(Player* player) const;
    bool hasOverThresholdItem() const;

    // there are players that killed the mob (instance only)
    InstanceLooters AllowedPlayers;

    private:
        void FillNotNormalLootFor(Player* player, bool presentAtLooting);
        QuestItemList* FillCurrencyLoot(Player* player);
        QuestItemList* FillFFALoot(Player* player);
        QuestItemList* FillQuestLoot(Player* player);
        QuestItemList* FillNonQuestNonFFANonCurrencyConditionalLoot(Player* player, bool presentAtLooting);

        std::set<uint64> PlayersLooting;

        QuestItemMap PlayerCurrencies;
        QuestItemMap PlayerQuestItems;
        QuestItemMap PlayerFFAItems;
        QuestItemMap PlayerNonQuestNonFFANonCurrencyConditionalItems;

        // All rolls are registered here. They need to know, when the loot is not valid anymore
        LootValidatorRefManager i_LootValidatorRefManager;
};

struct LootView
{
    Loot &loot;
    Player* viewer;
    PermissionTypes permission;

    LootView(Loot &_loot, Player* _viewer, PermissionTypes _permission = ALL_PERMISSION)
        : loot(_loot), viewer(_viewer), permission(_permission) { }

    void WriteData(ObjectGuid guid, LootType lootType, WorldPacket* data);
};

struct BonusLootData;

class BonusLootProcessor
{
    public:

        virtual void ProcessLootFor(Player* player);
        void SetGuaranteedLoot(bool value) { m_GuaranteedLoot = value; }

    protected:

        BonusLootProcessor() : m_GuaranteedLoot(false)
        { }

    protected:

        LootTemplate const* m_LootTemplate;
        bool m_GuaranteedLoot;

    private:

        void ProcessItemLootFor(Player* player);
        void ProcessCurrencyLootFor(Player* player);
};

class CreatureBonusLootProcessor : public BonusLootProcessor
{
    public:

        CreatureBonusLootProcessor(Creature* creature);

        void SendBonusLootToPlayers();

        void ProcessLootFor(Player* player);

    private:

        LootTemplate const* SelectLootTemplate();
        void GetPlayersWithBonus(std::list<Player*>& players, float range);

    private:

        Creature* m_Creature;
        BonusLootData const* m_BonusLootData;

    private:

        class PlayerWithCurrencyCheck
        {
            public:
                PlayerWithCurrencyCheck(WorldObject const* obj, float range, uint32 currencyId) : 
                    _obj(obj), _range(range), _currencyId(currencyId) {}
                bool operator()(Player* u);

            private:
                WorldObject const* _obj;
                float _range;
                uint32 _currencyId;
        };
};

class SpellBonusLootProcessor : public BonusLootProcessor
{
    public:

        SpellBonusLootProcessor(uint32 spellId);

        virtual void ProcessLootFor(Player* player);

    private:

        uint32 m_SpellId;
};

extern LootStore LootTemplates_Creature;
extern LootStore LootTemplates_Fishing;
extern LootStore LootTemplates_Gameobject;
extern LootStore LootTemplates_Item;
extern LootStore LootTemplates_Mail;
extern LootStore LootTemplates_Milling;
extern LootStore LootTemplates_Pickpocketing;
extern LootStore LootTemplates_Reference;
extern LootStore LootTemplates_Skinning;
extern LootStore LootTemplates_Disenchant;
extern LootStore LootTemplates_Prospecting;
extern LootStore LootTemplates_Spell;

void LoadLootTemplates_Creature();
void LoadLootTemplates_Fishing();
void LoadLootTemplates_Gameobject();
void LoadLootTemplates_Item();
void LoadLootTemplates_Mail();
void LoadLootTemplates_Milling();
void LoadLootTemplates_Pickpocketing();
void LoadLootTemplates_Skinning();
void LoadLootTemplates_Disenchant();
void LoadLootTemplates_Prospecting();

void LoadLootTemplates_Spell();
void LoadLootTemplates_Reference();

inline void LoadLootTables()
{
    LoadLootTemplates_Creature();
    LoadLootTemplates_Fishing();
    LoadLootTemplates_Gameobject();
    LoadLootTemplates_Item();
    LoadLootTemplates_Mail();
    LoadLootTemplates_Milling();
    LoadLootTemplates_Pickpocketing();
    LoadLootTemplates_Skinning();
    LoadLootTemplates_Disenchant();
    LoadLootTemplates_Prospecting();
    LoadLootTemplates_Spell();

    LoadLootTemplates_Reference();
}

#endif