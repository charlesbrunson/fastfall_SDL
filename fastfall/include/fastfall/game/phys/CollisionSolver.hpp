#pragma once

#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/Arbiter.hpp"

#include <array>
#include <deque>

#include "nlohmann/json_fwd.hpp"

namespace ff {

// struct with the arbiter and contact information prior to application
struct AppliedContact {
	Contact contact;
	ContactType type = ContactType::NO_SOLUTION;
};

//using CollisionFrame = std::vector<AppliedContact>;

// class for resolving a system of collisions on one collidable
class CollisionSolver {
private:
	struct CompResult {
		bool discardFirst = false;
		bool discardSecond = false;

		std::optional<Contact> contact;
		ContactType contactType = ContactType::NO_SOLUTION;
	};

	std::vector<Contact*> north_alt;
	std::vector<Contact*> south_alt;

	std::deque<Contact*> north;
	std::deque<Contact*> south;
	std::deque<Contact*> east;
	std::deque<Contact*> west;

	// the collidable we're solving for
	Collidable* collidable = nullptr;

	// whether solve() has been run yet
	size_t applyCounter = 0;

	// collision set of arbiters to solve
	std::vector<Contact*> contacts;
	std::deque<Contact> created_contacts;

	// arbiters that are determined invalid to moved here
	//std::vector<Contact*> discard;

	CompResult compare(const Contact* lhs, const Contact* rhs);
	CompResult pickV(const Contact* north, const Contact* south);
	CompResult pickH(const Contact* east, const Contact* west);

	void pushToAStack(std::vector<Contact*> contact);
	void pushToAStack(Contact* contact);

	// arbiter may be nullptr, returns true if any contact is applied
	bool apply(const Contact& contact, ContactType type = ContactType::SINGLE);
	bool applyStack(std::deque<Contact*>& stack);
	bool applyFirst(std::deque<Contact*>& stack);
	//bool applyAltStack(std::deque<Contact*>& altList);

	// returns true if any contact is applied
	bool solveX();
	bool solveY();

	bool canApplyAlt() const;

	nlohmann::ordered_json* json_dump = nullptr;

	void compareAll();

	void detectWedges();

	std::optional<Contact> detectWedge(const Contact* north, const Contact* south);

	std::vector<AppliedContact> frame;
public:
	enum class Ghost {
		NO_GHOST = 0,
		PARTIAL_GHOST = 1,
		FULL_GHOST = 2
	};

	// constructors
	CollisionSolver(Collidable* _collidable);

	// add an arbiter associated with the collidable to the collision set
	inline void pushContact(Contact* contact) { contacts.push_back(contact); };

	// attempts to resolve the combination of collisions
	// pushed contacts will be cleared after solving
	// returns vector of applied contact in order of application
	std::vector<AppliedContact> solve(nlohmann::ordered_json* dump_ptr = nullptr);

	static Ghost isGhostEdge(const Contact& basis, const Contact& candidate) noexcept;

};

}
