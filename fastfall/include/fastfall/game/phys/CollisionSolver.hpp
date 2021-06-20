#pragma once

#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/Arbiter.hpp"

#include <array>
#include <deque>

namespace ff {

// struct with the arbiter and contact information prior to application
struct AppliedContact {
	// contact state prior to application
	Contact contact;

	// this may be null depending on the type
	const Arbiter* arbiter = nullptr;
	const ColliderRegion* region = nullptr;

	ContactType type = ContactType::NO_SOLUTION;

};

using CollisionFrame = std::vector<AppliedContact>;

// class for resolving a system of collisions on one collidable
class CollisionSolver {
private:

	//std::array<std::list<Arbiter*>, 4u> arbPerDir;

	std::array<unsigned, 4> allCollisionCount;
	std::array<unsigned, 4> initialCollisionCount;
	std::array<unsigned, 4> appliedCollisionCount;

	std::deque<Arbiter*> north;
	std::deque<Arbiter*> south;
	std::deque<Arbiter*> east;
	std::deque<Arbiter*> west;

	// the collidable we're solving for
	Collidable* collidable = nullptr;

	// whether solve() has been run yet
	size_t applyCounter;

	//bool solved;

	// collision set of arbiters to solve
	std::vector<Arbiter*> arbiters;



	struct ArbCompResult {
		bool discardFirst = false;
		bool discardSecond = false;

		bool createdContact = false;
		Contact contact;
		ContactType contactType = ContactType::NO_SOLUTION;
	};

	// arbiters that are determined invalid to moved here
	std::vector<std::pair<Arbiter*, ArbCompResult>> discard;

	ArbCompResult compArbiters(const Arbiter* lhs, const Arbiter* rhs);

	ArbCompResult pickVArbiter(const Arbiter* north, const Arbiter* south);
	ArbCompResult pickHArbiter(const Arbiter* east, const Arbiter* west);

	// arbiter may be nullptr
	void apply(const Contact* contact, Arbiter* arbiter, ContactType type = ContactType::SINGLE);

	void updateArbiterStack(std::deque<Arbiter*>& stack);
	void applyArbiterStack(std::deque<Arbiter*>& stack);
	void applyArbiterFirst(std::deque<Arbiter*>& stack);
	void applyArbVertAsHori(std::deque<Arbiter*>& altList, std::deque<Arbiter*>& backupList);

	void solveX();
	void solveY();

	bool canApplyAltArbiters(std::deque<Arbiter*>& north_alt, std::deque<Arbiter*>& south_alt);


public:
	CollisionSolver(Collidable* _collidable);
	CollisionSolver(CollisionSolver&&) = default;
	CollisionSolver(const CollisionSolver&) = default;
	CollisionSolver& operator=(CollisionSolver&&) = default;
	CollisionSolver& operator=(const CollisionSolver&) = default;

	// add an arbiter associated with the collidable to the collision set
	inline void pushArbiter(Arbiter* arbiter) { arbiters.push_back(arbiter); };

	// attempts to resolve the combination of collisions
	// arbiters will be cleared after solving
	// resolution set will be populated
	// should be reinitialized after each solve
	void solve();

	// vector of AppliedArbiter in order of application
	CollisionFrame frame;

	// returns vector of each arbiter applied to resolve the collision set
	//inline const CollisionFrame* getCollisionFrame() { return &frame; }


	enum class Ghost {
		NO_GHOST = 0,
		PARTIAL_GHOST = 1,
		FULL_GHOST = 2
	};

	static Ghost isGhostEdge(const Contact* basis, const Contact* candidate) noexcept;

};

}