
#define NONIUS_RUNNER
#include "nonius/nonius.h++"

#include "lib.hpp"
#include "StringAtom.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stack>

#include <Eigen/Core>


using namespace OverLib::StringAtom;

namespace OverECS
{

class Context;

struct EntityID {
    uint32_t id;
    uint32_t generation;

    operator uint64_t ()
    {
        return generation << 31 || id;
    }

    bool operator == (const EntityID entity) const
    {
        return entity.id == id && entity.generation == generation;
    }
};

}


namespace std
{
template <> struct hash<OverECS::EntityID> {
    size_t operator()(const OverECS::EntityID& v) const
    {
        return v.id;
    }
};
}


namespace OverECS
{

class ComponentTable
{
public:
    ComponentTable()
    {
    }

    virtual void setContext(Context* context)
    {
        context_ = context;
    }

    Context* context_;
};

#define COMPONENTTABLE_METADATA(classname) \
    public: \
    static const Atom64 GetStaticName() { static const Atom64 STATIC_NAME = atomize64(#classname); return STATIC_NAME; }

struct AspectStorage {
    std::vector<Atom64> required_names_;
    std::vector<Atom64> optional_names_;
    std::unordered_set<EntityID> matching_entities_;
};

class Aspects
{
public:
    template<class... CT>
    static Aspects required()
    {
        Aspects ret;
        ret.required_names_.insert(ret.required_names_.end(), {CT::GetStaticName()...});
        return ret;
    }

//     template<class... CT>
//     Aspects& optional()
//     {
//         optional_names_.insert(optional_names_.end(), {CT::GetStaticName()...});
//         return *this;
//     }

    AspectStorage& connect(Context& context);

    std::vector<Atom64> required_names_;
    // std::vector<Atom64> optional_names_;
};
}


namespace std
{
template <> struct hash<std::vector<Atom64>> {
    size_t operator()(const std::vector<Atom64>& v) const
    {
        Atom64 ret = 0;
        for (Atom64 a : v) {
            ret += a;
        }
        return ret;
    }
};
}


namespace OverECS
{

class Context
{
    // Constructors
public:
    Context(size_t prealloc_entities = 512)
    {
        generations_.reserve(prealloc_entities);
        createEntity();                                     //  Entity 0 is reserved
    }

    //  Setup methods
public:
    template<class CT>
    bool registerComponentTable(CT& component_table)
    {
        if (component_tables_.find(CT::GetStaticName()) != component_tables_.end()) {
            return false;
        }
        component_tables_.emplace(CT::GetStaticName(),  component_table);
        component_table.setContext(this);
        return true;
    };

//  Interface
public:
    ComponentTable* getComponentTable(Atom64 name)
    {
        auto found = component_tables_.find(name);
        if (found ==  component_tables_.end()) {
            return nullptr;
        }
        return &found->second;
    }

    template<class CTType>
    CTType* getComponentTable()
    {
        return static_cast<CTType*>(getComponentTable(CTType::GetStaticName()));
    }

    EntityID createEntity()
    {
        if (free_entities_.size() > 0) {
            uint32_t entID = free_entities_.top();
            free_entities_.pop();
            return {entID, generations_[entID]};
        } else {
            generations_.push_back(1);
            uint32_t id = generations_.size() - 1;
            return {id,  1};
        }
    }

    void destroyEntity(EntityID& entity)
    {
        uint32_t generation = generations_[entity.id];
        if (entity.generation ==  generation) {
            performEntityDestructionCallbacks(entity);
            generations_[entity.id] = generation + 1;
            free_entities_.push(entity.id);
        } else {
            // invalid EntityID, invalidate it regardless
            if (entity.generation != 0) {
                //  TODO:  Log attempt to destroy an invalid EntityID
            }
        }
        entity.generation = 0;
    }

//     bool registerEntityDestructionCB(EntityID entity, std::function<void(EntityID)>& cb)
//     {
//         if (entity.generation != generations_[entity.id]) {
//             return false;
//         }
//         if (entity_destruction_cbs_.size() >= entity.id) {
//             entity_destruction_cbs_.resize(entity.id + 1);
//         }
//         entity_destruction_cbs_[entity.id].emplace_back(cb);
//         return true;
//     }

    bool registerEntityDestructionVector(EntityID entity, std::vector<uint32_t>& target)
    {
        if (entity.generation != generations_[entity.id]) {
            return false;
        }
        if (entity_destruction_vectors_.size() <= entity.id) {
            entity_destruction_vectors_.resize(entity.id + 1);
        }
        entity_destruction_vectors_[entity.id].emplace(&target);
        return true;
    }

    bool registerEntityDestructionUMap(EntityID entity, std::unordered_map<uint32_t, uint32_t>& target)
    {
        if (entity.generation != generations_[entity.id]) {
            return false;
        }
        if (entity_destruction_umaps_.size() <= entity.id) {
            entity_destruction_umaps_.resize(entity.id + 1);
        }
        entity_destruction_umaps_[entity.id].emplace(&target);
        return true;
    }

//     bool unregisterEntityDestructionCB(EntityID entity, std::function<void(EntityID)>& cb)
//     {
//         if (entity.generation != generations_[entity.id]) {
//             return false;
//         }
//         if (entity_destruction_cbs_.size() >= entity.id) {
//             return false;
//         }
//         auto &deq = entity_destruction_cbs_[entity.id];
//         auto found = deq.find(cb)
//     }

    bool unregisterEntityDestructionVector(EntityID entity, std::vector<uint32_t>& target)
    {
        if (entity.generation != generations_[entity.id]) {
            return false;
        }
        if (entity_destruction_vectors_.size() >= entity.id) {
            return false;
        }
        auto& set = entity_destruction_vectors_[entity.id];
        auto found = set.find(&target);
        if (set.end() == found) {
            return false;
        }
        set.erase(found);
        return true;
    }

    bool unregisterEntityDestructionUMap(EntityID entity, std::unordered_map<uint32_t, uint32_t>& target)
    {
        if (entity.generation != generations_[entity.id]) {
            return false;
        }
        if (entity_destruction_umaps_.size() >= entity.id) {
            return false;
        }
        auto& set = entity_destruction_umaps_[entity.id];
        auto found = set.find(&target);
        if (set.end() == found) {
            return false;
        }
        set.erase(found);
        return true;
    }

    void performEntityDestructionCallbacks(EntityID entity)
    {
        if (entity_destruction_vectors_.size() < entity.id) {
            auto vecs = entity_destruction_vectors_[entity.id];
            for (std::vector<uint32_t>* vec : vecs) {
                (*vec)[entity.id] = 0;
            }
            vecs.clear();
        }
        if (entity_destruction_umaps_.size() < entity.id) {
            auto maps = entity_destruction_umaps_[entity.id];
            for (std::unordered_map<uint32_t, uint32_t>* map : maps) {
                map->erase(entity.id);
            }
            maps.clear();
        }
        entity_components.erase(entity.id);
    }

    void performComponentAdded(EntityID entity, Atom64 name)
    {
//         std::cout << entity.id << ":" << deatomize64(name) << std::endl;
        entity_components.emplace(entity.id, name);
        auto comp_range = entity_components.equal_range(entity.id);
        auto aspect_range = aspect_listeners_.equal_range(name);
        for (auto it = aspect_range.first; it != aspect_range.second; ++it) {
            bool successA = true;
            for (Atom64 a : it->second.required_names_) {
                bool successB = false;
                for (auto itb = comp_range.first; itb != comp_range.second; ++itb) {
                    Atom64 b = itb->second;
//                     std::cout << entity.id << " " << deatomize64(a) <<  " == " << deatomize64(b) << " -> " << (a == b)<< std::endl;
                    if (a == b) {
                        successB = true;
                        break;
//                     } else if (b > a) {
//                         break;
                    }
                }
                if (!successB) {
                    successA = false;
                    break;
                }
            }
            if (successA) {
                it->second.matching_entities_.emplace(entity);
//              std::cout << entity.id << ":added:" << it->second.matching_entities_.size() << std::endl;
            }
        }
    }

    void performComponentRemoved(EntityID entity, Atom64 name)
    {
        auto comp_range = entity_components.equal_range(entity.id);
        auto aspect_range = aspect_listeners_.equal_range(name);

        for (auto it = aspect_range.first; it != aspect_range.second; ++it) {
            bool successA = true;
            for (Atom64 a : it->second.required_names_) {
                bool successB = false;
                for (auto itb = comp_range.first; itb != comp_range.second; ++itb) {
                    Atom64 b = itb->second;
                    if (a == b) {
                        successB = true;
                        break;
//                     } else if (a > b) {
//                         break;
                    }
                }
                if (!successB) {
                    successA = false;
                    break;
                }
            }
            if (successA) {
                it->second.matching_entities_.erase(entity);
            }
        }

        for (auto it = comp_range.first; it != comp_range.second; ++it) {
            if (it->second == name) {
                entity_components.erase(it);
                break;
            }
        }
    }

    const std::unordered_map<Atom64, ComponentTable&>& getComponentTables()
    {
        return component_tables_;
    }

    AspectStorage& getOrCreateAspectStorage(Aspects& aspects)
    {
        // Currently not using optional_names_ in this build...
        auto found = aspect_storage_.find(aspects.required_names_);
        if (found ==  aspect_storage_.end()) {
            aspect_storage_.emplace(aspects.required_names_, std::unique_ptr<AspectStorage>(new AspectStorage));
            AspectStorage& storage = *aspect_storage_[aspects.required_names_];
//             std::cout << "floo" << &storage << std::endl;
            storage.required_names_.swap(aspects.required_names_);
            //storage.optional_names_.swap(aspects.optional_names_);
            for (Atom64 name : storage.required_names_) {
                aspect_listeners_.emplace(name, storage);
            }
            return storage;
        } else {
            return *found->second;
        }
    }

private:

public:
    std::vector<uint32_t> generations_;
    std::stack<uint32_t> free_entities_;
    std::unordered_map<Atom64, ComponentTable&> component_tables_;
//     std::vector<std::deque<std::function<void(EntityID)>>> entity_destruction_cbs_;
    std::vector<std::set<std::vector<uint32_t>*>> entity_destruction_vectors_;
    std::vector<std::set<std::unordered_map<uint32_t, uint32_t>*>> entity_destruction_umaps_;

    std::unordered_multimap<uint32_t, Atom64> entity_components;
    std::unordered_map<std::vector<Atom64>, std::unique_ptr<AspectStorage>> aspect_storage_;
    std::unordered_multimap<Atom64, AspectStorage&> aspect_listeners_;
};

AspectStorage& Aspects::connect(OverECS::Context& context)
{
    required_names_.shrink_to_fit();
    //optional_names_.shrink_to_fit();
    std::sort(required_names_.begin(), required_names_.end());
    //std::sort(optional_names_.begin(), optional_names_.end());
    return context.getOrCreateAspectStorage(*this);
}


/* // TODO:  Re-create the EventManager system
class EventManager : public ComponentTable
{
    COMPONENTTABLE_METADATA(EventManager);

public:
    EventManager(size_t prealloc_callbacks)
        : ComponentTable()
    {
    }
};
*/

}


namespace pos_vel
{
using namespace OverECS;

class PosComp : public ComponentTable
{
    COMPONENTTABLE_METADATA(PosComp);
public:
    PosComp(size_t prealloc_entities = 512)
        : ComponentTable()
    {
        positions_.reserve(prealloc_entities);
        generations_.reserve(prealloc_entities);
    }

    // Interface
public:
    // operator[] is, as usual in std, unchecked, be careful
    Position& operator[](EntityID entity)
    {
        return positions_[entity.id];
    }

    const Position operator[](EntityID entity) const
    {
        return positions_[entity.id];
    }

    //  These are the checked versions of above
    Position* get(EntityID entity)
    {
        if (entity.generation == 0) return nullptr;
        if (positions_.size() >= entity.id) return nullptr;
        if (generations_[entity.id] !=  entity.generation) return nullptr;
        return &positions_[entity.id];
    }

    Position* getOrCreate(EntityID entity)
    {
        return getOrCreate(entity, {0.0f, 0.0f});
    }

    Position* getOrCreate(EntityID entity, const Position newPos)
    {
        //  If invalid entity
        if (entity.generation == 0) return nullptr;
        if (positions_.size() <= entity.id) {
            positions_.resize(entity.id + 1, {0.0f, 0.0f});
            generations_.resize(entity.id + 1,  0);
        }
        uint32_t gen = generations_[entity.id];
        if (gen !=  entity.generation) {
            if (gen != 0) context_->unregisterEntityDestructionVector({entity.id, gen}, generations_);
            gen = entity.generation;
            context_->registerEntityDestructionVector(entity, generations_);
            context_->performComponentAdded(entity, GetStaticName());
        }
        Position& pos = positions_[entity.id];
        pos = newPos;
        return &pos;
    }

protected:
    // Vector due to it being dense
    std::vector<Position> positions_;
    std::vector<uint32_t> generations_;
};
/*
class VelComp : public ComponentTable
{
    COMPONENTTABLE_METADATA(VelComp);
public:
    VelComp(size_t prealloc_entities = 512)
        : ComponentTable()
    {
        velocities_.reserve(prealloc_entities);
        generations_.reserve(prealloc_entities);
    }

// Interface
public:
    // operator[] is, as usual in std, unchecked, be careful
    Velocity& operator[](EntityID entity)
    {
        return velocities_[entity.id];
    }

    const Velocity operator[](EntityID entity) const
    {
        auto found = velocities_.find(entity.id);
        if (velocities_.end() == found) return {0.0f, 0.0f};
        return found->second;
    }

    //  These are the checked versions of above
    Velocity* get(EntityID entity)
    {
        if (entity.generation == 0) return nullptr;
        auto found = velocities_.find(entity.id);
        if (velocities_.end() == found) return nullptr;
        if (generations_[entity.id] != entity.generation) return nullptr;
        return &found->second;
    }

    Velocity* getOrCreate(EntityID entity)
    {
        return getOrCreate(entity, {0.0f, 0.0f});
    }

    Velocity* getOrCreate(EntityID entity, Velocity newVel)
    {
        //  If invalid entity
        if (entity.generation == 0) return nullptr;
        auto found = velocities_.find(entity.id);
        if (velocities_.end() == found) {
            generations_[entity.id] = entity.generation;
            Velocity& vel = velocities_[entity.id];
            vel = newVel;
            context_->registerEntityDestructionUMap(entity, generations_);
            context_->performComponentAdded(entity, GetStaticName());
            return &vel;
        } else {
            uint32_t& gen = generations_[entity.id];
            if (gen != entity.generation) {
                if (gen != 0) context_->unregisterEntityDestructionUMap({entity.id, gen}, generations_);
                gen = entity.generation;
                context_->registerEntityDestructionUMap(entity, generations_);
                context_->performComponentAdded(entity, GetStaticName());
            }
            return &velocities_[entity.id];
        }
    }

protected:
    // Unordered_map due to it being sparse (even though it is not really sparse in this example as they are all packed near the front...,  being fair)
    std::unordered_map<uint32_t, Velocity> velocities_;
    std::unordered_map<uint32_t, uint32_t> generations_;
};
/*/
class VelComp : public ComponentTable
{
    COMPONENTTABLE_METADATA(VelComp);
public:
    VelComp(size_t prealloc_entities = 512)
        : ComponentTable()
    {
        velocities_.reserve(prealloc_entities);
        generations_.reserve(prealloc_entities);
    }

    // Interface
public:
    // operator[] is, as usual in std, unchecked, be careful
    Velocity& operator[](EntityID entity)
    {
        return velocities_[entity.id];
    }

    const Velocity operator[](EntityID entity) const
    {
        return velocities_[entity.id];
    }

    //  These are the checked versions of above
    Velocity* get(EntityID entity)
    {
        if (entity.generation == 0) return nullptr;
        if (velocities_.size() >= entity.id) return nullptr;
        if (generations_[entity.id] !=  entity.generation) return nullptr;
        return &velocities_[entity.id];
    }

    Velocity* getOrCreate(EntityID entity)
    {
        return getOrCreate(entity, {0.0f, 0.0f});
    }

    Velocity* getOrCreate(EntityID entity, const Velocity newVel)
    {
        //  If invalid entity
        if (entity.generation == 0) return nullptr;
        if (velocities_.size() <= entity.id) {
            velocities_.resize(entity.id + 1, {0.0f, 0.0f});
            generations_.resize(entity.id + 1,  0);
        }
        uint32_t gen = generations_[entity.id];
        if (gen !=  entity.generation) {
            if (gen != 0) context_->unregisterEntityDestructionVector({entity.id, gen}, generations_);
            gen = entity.generation;
            context_->registerEntityDestructionVector(entity, generations_);
            context_->performComponentAdded(entity, GetStaticName());
        }
        Velocity& vel = velocities_[entity.id];
        vel = newVel;
        return &vel;
    }

protected:
    // Vector due to it being dense
    std::vector<Velocity> velocities_;
    std::vector<uint32_t> generations_;
};
// */

void build(Context& context)
{
    PosComp* positions = context.getComponentTable<PosComp>();
    VelComp* velocities = context.getComponentTable<VelComp>();

    // setup entities
    for (int i = 0; i < N_POS_VEL; ++i) {
        EntityID entity = context.createEntity();
        positions->getOrCreate(entity, {0.0f, 0.0f});
        velocities->getOrCreate(entity, {0.0f, 0.0f});
    }

    for (int i = 0; i < N_POS; ++i) {
        EntityID entity = context.createEntity();
        positions->getOrCreate(entity, {0.0f, 0.0f});
    }
}

}


namespace parallel
{
using namespace OverECS;

void build(Context& context)
{
}

}


namespace pos_vel_eigen
{
using namespace OverECS;
typedef Eigen::Vector2f Position;
typedef Eigen::Vector2f Velocity;
typedef Eigen::Matrix2Xf Positions;
typedef Eigen::Matrix2Xf Velocities;
typedef Positions::ColXpr PositionRef;
typedef Velocities::ColXpr VelocityRef;

class PosComp : public ComponentTable
{
    COMPONENTTABLE_METADATA(PosComp);
public:
    PosComp(size_t prealloc_entities = 512)
        : ComponentTable()
        , positions_(2, prealloc_entities)
    {
        positions_.fill(0.0f);
        generations_.reserve(prealloc_entities);
    }

    // Interface
public:
    // operator[] is, as usual in std, unchecked, be careful
    PositionRef operator[](EntityID entity)
    {
        return positions_.col(entity.id);
    }

    const Position operator[](EntityID entity) const
    {
        return positions_.col(entity.id);
    }

    //  These are the checked versions of above
    PositionRef get(EntityID entity)
    {
        if (entity.generation == 0) return positions_.col(0);
        if (positions_.size() >= entity.id) return positions_.col(0);
        if (generations_[entity.id] !=  entity.generation) return positions_.col(0);
        return positions_.col(entity.id);
    }

    PositionRef getOrCreate(EntityID entity)
    {
        return getOrCreate(entity, {0.0f, 0.0f});
    }

    PositionRef getOrCreate(EntityID entity, const Position newPos)
    {
        //  If invalid entity
        if (entity.generation == 0) return positions_.col(0);
        int oldSize = positions_.cols();
        if (oldSize <= entity.id) {
            int newSize = (entity.id + 1) * 1.24;
            positions_.conservativeResize(2, newSize);
            positions_.rightCols(newSize - oldSize).fill(0.0f);
            generations_.resize(entity.id + 1,  0);
        }
        uint32_t gen = generations_[entity.id];
        if (gen !=  entity.generation) {
            if (gen != 0) context_->unregisterEntityDestructionVector({entity.id, gen}, generations_);
            gen = entity.generation;
            context_->registerEntityDestructionVector(entity, generations_);
            context_->performComponentAdded(entity, GetStaticName());
        }
        PositionRef pos = positions_.col(entity.id);
        pos = newPos;
        return pos;
    }

protected:
public:
    // Vector due to it being dense
    Positions positions_;
    std::vector<uint32_t> generations_;
};

class VelComp : public ComponentTable
{
    COMPONENTTABLE_METADATA(VelComp);
public:
    VelComp(size_t prealloc_entities = 512)
        : ComponentTable()
        , velocities_(2, prealloc_entities)
    {
        velocities_.fill(0.0f);
        generations_.reserve(prealloc_entities);
    }

    // Interface
public:
    // operator[] is, as usual in std, unchecked, be careful
    VelocityRef operator[](EntityID entity)
    {
        return velocities_.col(entity.id);
    }

    const Velocity operator[](EntityID entity) const
    {
        return velocities_.col(entity.id);
    }

    //  These are the checked versions of above
    VelocityRef get(EntityID entity)
    {
        if (entity.generation == 0) return velocities_.col(0);
        if (velocities_.size() >= entity.id) return velocities_.col(0);
        if (generations_[entity.id] !=  entity.generation) return velocities_.col(0);
        return velocities_.col(entity.id);
    }

    VelocityRef getOrCreate(EntityID entity)
    {
        return getOrCreate(entity, {0.0f, 0.0f});
    }

    VelocityRef getOrCreate(EntityID entity, const Position newPos)
    {
        //  If invalid entity
        if (entity.generation == 0) return velocities_.col(0);
        auto oldSize = velocities_.cols();
        if (oldSize <= entity.id) {
            int newSize = (entity.id + 1) * 1.24;
            velocities_.conservativeResize(2, newSize);
            velocities_.rightCols(newSize - oldSize).fill(0.0f);
            generations_.resize(entity.id + 1,  0);
        }
        uint32_t gen = generations_[entity.id];
        if (gen !=  entity.generation) {
            if (gen != 0) context_->unregisterEntityDestructionVector({entity.id, gen}, generations_);
            gen = entity.generation;
            context_->registerEntityDestructionVector(entity, generations_);
            context_->performComponentAdded(entity, GetStaticName());
        }
        VelocityRef pos = velocities_.col(entity.id);
        pos = newPos;
        return pos;
    }

protected:
public:
    // Vector due to it being dense
    Velocities velocities_;
    std::vector<uint32_t> generations_;
};

void build(Context& context)
{
    PosComp* positions = context.getComponentTable<PosComp>();
    VelComp* velocities = context.getComponentTable<VelComp>();

    // setup entities
    for (int i = 0; i < pos_vel::N_POS_VEL; ++i) {
        EntityID entity = context.createEntity();
        positions->getOrCreate(entity, {0.0f, 0.0f});
        velocities->getOrCreate(entity, {0.0f, 0.0f});
    }

    for (int i = 0; i < pos_vel::N_POS; ++i) {
        EntityID entity = context.createEntity();
        positions->getOrCreate(entity, {0.0f, 0.0f});
    }
}

}


NONIUS_BENCHMARK("pos_vel build_destroy", [](nonius::chronometer meter)
{
    using namespace OverECS;
    using namespace pos_vel;

    meter.measure([] {
        /*
        size_t prealloc_pos = N_POS_VEL + N_POS;
        size_t prealloc_vel = N_POS_VEL * 2;
        PosComp positions(prealloc_pos);
        VelComp velocities(prealloc_vel);
        Context context(prealloc_pos);
        /*/
        PosComp positions;
        VelComp velocities;
        Context context;
        //*/
        context.registerComponentTable(positions);
        context.registerComponentTable(velocities);
        build(context);
        // Do not optimize out the above
        return context.getComponentTables().size() + positions[{1, 1}].x + velocities[{1, 1}].dx;
    });
})

NONIUS_BENCHMARK("pos_vel update", [](nonius::chronometer meter)
{
    using namespace OverECS;
    using namespace pos_vel;
    PosComp positions;
    VelComp velocities;
    Context context;
    context.registerComponentTable(positions);
    context.registerComponentTable(velocities);
    AspectStorage& posvelStorage = Aspects::required<PosComp, VelComp>().connect(context);
    AspectStorage& posStorage = Aspects::required<PosComp>().connect(context);
    build(context);
    float ret = 0;
    int retpv = 0;
    int retp = 0;
    meter.measure([&] {
        for (auto entity : posvelStorage.matching_entities_)
        {
            Position& pos = positions[entity];
//             pos.x += pos.y;
//             pos.y += pos.x
            Velocity& vel = velocities[entity];
            pos.x += vel.dx;
            pos.y += vel.dy;
//             ++retpv;
        }
        for (auto entity : posStorage.matching_entities_)
        {
            Position& pos = positions[entity];
            ret += pos.x;
//              ++retp;
        }
//         return ret+retpv+retp;
    });
    //std::cout << "ret:" << ret << " pv:" << retpv << " p:" << retp << std::endl;
})


NONIUS_BENCHMARK("pos_vel_eigen build_destroy", [](nonius::chronometer meter)
{
    using namespace OverECS;
    using namespace pos_vel_eigen;

    meter.measure([] {
        /*
        size_t prealloc_pos = N_POS_VEL + N_POS;
        size_t prealloc_vel = N_POS_VEL * 2;
        PosComp positions(prealloc_pos);
        VelComp velocities(prealloc_vel);
        Context context(prealloc_pos);
        /*/
        PosComp positions;
        VelComp velocities;
        Context context;
        //*/
        context.registerComponentTable(positions);
        context.registerComponentTable(velocities);
        build(context);
        // Do not optimize out the above
        return context.getComponentTables().size();
    });
})

NONIUS_BENCHMARK("pos_vel_eigen update", [](nonius::chronometer meter)
{
    using namespace OverECS;
    using namespace pos_vel_eigen;
    PosComp positions;
    VelComp velocities;
    Context context;
    context.registerComponentTable(positions);
    context.registerComponentTable(velocities);
    AspectStorage& posvelStorage = Aspects::required<PosComp, VelComp>().connect(context);
    AspectStorage& posStorage = Aspects::required<PosComp>().connect(context);
    build(context);
//     std::cout << "Done P:" << positions.positions_.cols() << "  V" << velocities.velocities_.cols() << std::endl;
    float ret = 0;
    int retpv = 0;
    int retp = 0;
//     std::cout << "p" << positions.positions_.cols() << std::endl;
    //std::cout << "v" << velocities.velocities_.cols() << std::endl;
    meter.measure([&] {
        positions.positions_.leftCols(velocities.velocities_.cols()).noalias() += velocities.velocities_;
    });
    //std::cout << "ret:" << ret << " pv:" << retpv << " p:" << retp << std::endl;
})
