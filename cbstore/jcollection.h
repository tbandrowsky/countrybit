#pragma once

#include <vector>
#include <list>
#include <fstream>
#include <algorithm>
#include "skip_list.h"
#include <thread>
#include <atomic>

#define DEBUG_CLAIM_CHECKING 0

namespace countrybit
{
	namespace database
	{

		class jcollection
		{

		public:

			int clause_count = 0;
			int reduce_count = 0;
			int rule_count = 0;
			int cdcl_rule_count = 0;
			int node_count = 0;
			int max_node_length = 0;

			void clear_counts()
			{
				clause_count = 0;
				reduce_count = 0;
				rule_count = 0;
				cdcl_rule_count = 0;
				node_count = 0;
				max_node_length = 0;
			}

		private:

			rule_node root;
			std::vector<literal_claim> literal_claims;
			int max_literal;

			void find_matches(rule_node* root, rule_node* p, literal candidate, std::vector<rule_node*>& matches, bool trace)
			{
				literal lnc = literal_traits::get_literal_name(candidate);
				//				std::cout << "start matching:" << candidate << std::endl;

				if (p != root) {
					if (p->max_child < lnc && p->max_child > 0) {
#if DEBUG_CLAIM_CHECKING
						std::cout << "c no match:" << p->to_string() << " with " << candidate << std::endl;
#endif
						return;
					}
					else if (p->children.size() == 0) {
#if DEBUG_CLAIM_CHECKING
						std::cout << "c matches:" << p->to_string() << " with " << candidate << std::endl;
#endif
						for (auto px = p; px != root; px = px->parent) {
							if (px->ln == lnc) {
								matches.push_back(p);
								break;
							}
						}

						return;
					}
				}

				for (auto nodeid : p->children)
				{
					literal l = nodeid.first.l;
					literal ln = nodeid.first.ln;
					auto& lc = literal_claims[ln];
					bool hasit = false;
					if (ln <= lnc) {
#if DEBUG_CLAIM_CHECKING
						std::cout << "c find :" << p->to_string() << "[ " << ln << "] with " << lnc << " " << nodeid.second->max_child << std::endl;
#endif
						hasit = (lc.guess == l && lnc != ln) || (l == candidate);
						if (hasit)
						{
							find_matches(root, nodeid.second, candidate, matches, trace);
						}
					}
					else {
						break;
					}

				}

			}

			bool has_matches(rule_node* root, rule_node* p, literal candidate)
			{
				literal lnc = literal_traits::get_literal_name(candidate);

				if (p != root) {
					if (p->max_child < lnc && p->max_child > 0) {
#if DEBUG_CLAIM_CHECKING
						std::cout << "c no match:" << p->to_string() << " with " << candidate << std::endl;
#endif
						return false;
					}
					else if (p->children.size() == 0) {
#if DEBUG_CLAIM_CHECKING
						std::cout << "c matches:" << p->to_string() << " with " << candidate << std::endl;
#endif
						return true;
					}
				}

				for (auto nodeid : p->children)
				{
					literal ln = nodeid.first.ln;
					node_count++;
					if (ln <= lnc) {
						literal l = nodeid.first.l;
						auto& lc = literal_claims[ln];
						bool hasit = false;
#if DEBUG_CLAIM_CHECKING
						std::cout << "c has :" << p->to_string() << "[ " << ln << "] with " << lnc << " " << nodeid.second->max_child << std::endl;
#endif
						hasit = (lc.guess == l && lnc != ln) || (l == candidate);
						if (hasit)
						{
							bool r = has_matches(root, nodeid.second, candidate);
							if (r) {
								return r;
							}
						}
					}
					else {
						break;
					}
				}

				return false;
			}

			literal_name next_conflict(rule_node* root, rule_node* p, literal_name lnc)
			{
				if (p != root) {
					if (p->max_child < lnc && p->max_child > 0) {
#if DEBUG_CLAIM_CHECKING
						std::cout << "c no match:" << p->to_string() << " with " << candidate << std::endl;
#endif
						return literal_traits::null_literal;
					}
					else if (p->children.size() == 0) {
#if DEBUG_CLAIM_CHECKING
						std::cout << "c matches:" << p->to_string() << " with " << candidate << std::endl;
#endif
						return p->ln;
					}
				}

				for (auto nodeid : p->children)
				{
					literal ln = nodeid.first.ln;
					node_count++;
					if (ln > lnc) {
						literal l = nodeid.first.l;
						auto& lc = literal_claims[ln];
						bool hasit = false;
#if DEBUG_CLAIM_CHECKING
						std::cout << "c has :" << p->to_string() << "[ " << ln << "] with " << lnc << " " << nodeid.second->max_child << std::endl;
#endif
						hasit = lc.guess == l;
						if (hasit)
						{
							literal_name r = next_conflict(root, nodeid.second, literal_traits::null_literal);
							if (r) {
								return p->ln;
							}
						}
					}
					else {
						break;
					}
				}

				return literal_traits::null_literal;
			}

			void find_reduction_candidates(rule_node* root, rule_node* p, rule& r, std::vector<rule_node*>& reductions)
			{
				for (auto nodeid : p->children)
				{
					literal l = nodeid.first.l;
					literal ln = nodeid.first.ln;
					if (r[ln] != std::end(r))
					{
						find_reduction_candidates(root, nodeid.second, r, reductions);
					}
				}

				if (p != root && p->children.size() == 0)
				{
					reductions.push_back(p);
				}
			}

			rule_node* find_existing(rule_node* root, rule_node* p, int depth, rule& r)
			{
				rule_node* exists = nullptr;

				for (auto ri : r) {
					auto pci = p->children[literal_pair(ri.second)];
					if (pci != std::end(p->children)) {
						p = pci->second;
					}
				}

				if (p != root && p->children.size() == 0)
				{
					exists = p;
				}

				return exists;
			}

			rule_node* find_existing_old(rule_node* root, rule_node* p, int depth, rule& r)
			{
				rule_node* exists = nullptr;

				for (auto nodeid : p->children)
				{
					literal l = nodeid.first.l;
					literal ln = nodeid.first.ln;
					bool hasit = r.has(ln, [l](auto& lx) { return lx == l; });
					if (hasit)
					{
						exists = find_existing(root, nodeid.second, depth + 1, r);
						if (exists) {
							return exists;
						}
					}
				}

				if (p != root && p->children.size() == 0 && depth <= r.size())
				{
					exists = p;
				}
				return exists;
			}

			literal_name reduces_immediate(rule_node* root, rule_node* r, rule& s)
			{
				literal_name reduction_literal = literal_traits::null_literal;
				int reduction_count = 0;

				while (r != root)
				{
					auto ln = r->ln;
					auto si = s.at(ln);
					if (si != s.end()) {
						literal sl = si.get_value();
						if (sl != r->l) {
							reduction_count++;
							if (reduction_count > 1) {
								return literal_traits::null_literal;
							}
							else
							{
								reduction_literal = sl;
							}
						}
					}
					else
					{
						return literal_traits::null_literal;
					}
					r = r->parent;
				}

				if (reduction_literal != literal_traits::null_literal)
				{
					//	r->deleted = true;
	//					std::cout << "c reduce find:" << *r << " -> " << s << " on " << reduction_literal << std::endl;
				}

				return reduction_literal;
			}

		public:

			simd_vector assignments;

			std::ofstream proofout;

			jcollection() : max_literal(0)
			{
				proofout.open("proof.out", std::ios::trunc);
			}

			~jcollection()
			{
			}

			void statement_to_rule(statement& s, rule& sr)
			{
				for (auto l : s)
				{
					literal_name ln = literal_traits::get_literal_name(l);
					sr.insert_or_assign(ln, l);
				}
			}

			bool exists(rule_node* root, rule& sr)
			{
				if (sr.size() == 0)
					return true;

				bool exists = false;

				exists = find_existing(root, root, 0, sr);

				return exists;
			}

			bool exists(statement& s)
			{
				if (s.size() == 0)
					return true;

				bool does_exist = false;

				rule sr;

				statement_to_rule(s, sr);
				does_exist = exists(&root, sr);

				return does_exist;
			}

			literal_name reduces(statement st)
			{
				rule temp;
				literal_name red = literal_traits::null_literal;
				statement_to_rule(st, temp);
				literal_name ln = reduces(&root, temp);
				return ln;
			}

			void resize(int _max_literal)
			{
				max_literal = _max_literal;
				literal_claims.resize(_max_literal);
				assignments.dim(_max_literal);
			}

			bool conflicts(rule* r, clause* c)
			{
				for (auto rk : *r)
				{
					auto l = rk.second;
					if (!c->find_guess(l))
						return false;
				}

				return true;
			}

			literal_name next_conflict(literal_name lnc)
			{
				return next_conflict(&root, &root, lnc);
			}

			literal reduces(rule_node* r, rule& sr)
			{
				if (sr.size() == 0)
					return literal_traits::null_literal;

				std::vector<rule_node*> reductions;
				find_reduction_candidates(r, r, sr, reductions);
				for (auto rx : reductions)
				{
					literal l = reduces_immediate(r, rx, sr);
					if (l != literal_traits::null_literal)
						return l;
				}

				return literal_traits::null_literal;
			}

			struct create_rule_result {
				bool created;
				bool empty_set;

				create_rule_result() : created(false), empty_set(false) { }
			};

			create_rule_result create_rule(statement& st, bool trace)
			{
				rule temp;
				statement_to_rule(st, temp);
				auto crr = create_rule_impl(&root, temp, trace);
				return crr;
			}

			void calculate_node(rule_node* p)
			{
				literal_name max_child = literal_traits::null_literal;

				for (auto nodeid : p->children)
				{
					if (nodeid.second->max_child >= max_child)
					{
						max_child = nodeid.second->max_child;
					}
					else
					{
						std::cout << "c " << nodeid.second->to_string_full() << " ! Looks like your tree sucks." << std::endl;
						abort();
					}
				}

				p->max_child = max_child;

				if (p->parent != &root) {
					calculate_node(p->parent);
				}
			}

			literal get_backjump_guess(literal_name minimum)
			{
				auto p = &root;
				auto min_ln = literal_traits::get_literal_name(minimum);

				int distance = -1;
				literal guess_literal = literal_traits::null_literal;

				for (auto nodei : p->children)
				{
					if (nodei.first.ln > min_ln)
					{
						int new_distance = nodei.second->max_child - nodei.first.ln;
						if (distance == -1 || distance > new_distance) {
							guess_literal = nodei.first.l;
							distance = new_distance;
						}
#if DEBUG_CLAIM_CHECKING
						std::cout << "c " << nodei.second->to_string() << " distance " << distance << std::endl;
#endif
					}
				}

				return literal_traits::get_anti_literal_value(guess_literal);
			}

			create_rule_result create_rule_impl(rule_node* root, rule& rst, bool trace)
			{
				create_rule_result result;
				result.empty_set = false;
				result.created = false;
				clause_count = 0;
				node_count = 0;

				if (!exists(root, rst))
				{
					result.created = true;
					rule_count++;
					literal lred = reduces(root, rst);
					if (lred)
					{
						reduce_count++;
						rule stred;
						for (auto l : rst)
						{
							if (l.second != lred) {
								stred.insert_or_assign(l.first, l.second);
							}
						}
						if (trace)
						{
							std::cout << "c reduction:" << rst << " on " << lred << " to " << stred << std::endl;
							//	proofout << "d " <<  st << " 0" << std::endl;
						}
						if (stred.size() > 0) {
							result.empty_set = create_rule_impl(root, stred, trace).empty_set;
						}
						else {
							if (trace)
							{
								std::cout << "c is empty set, unsatisfiable" << std::endl;
							}
							result.empty_set = true;
						}
					}
					else {
						rule_node* p = root;
						rule_node* cut_p = nullptr;
						literal cut_l = literal_traits::null_literal;
						literal_name last = literal_traits::null_literal;

						for (auto l : rst)
						{
							last = l.first;
							auto& clitem = literal_claims[l.first];
							if (clitem.claimant) {
								clitem.claimant->clear();
								clitem.guess = literal_traits::null_literal;
							}
							auto nodei = p->children[l.second];
							if (nodei != p->children.end())
							{
								cut_l = l.second;
								p = nodei->second;
								cut_p = p;
							}
							else
							{
								auto* rn = new rule_node();
								rn->l = l.second;
								rn->ln = l.first;
								rn->max_child = rn->ln;
								rn->parent = p;
								p->children.insert_or_assign(l.second, rn);
								p = rn;
								cut_l = literal_traits::null_literal;
								cut_p = nullptr;
							}
						}

						if (trace)
						{
							//						proofout << *r <<  " 0" << std::endl;
							std::cout << "c added rule:" << p->to_string() << std::endl;
						}

						if (cut_p)
						{
							cut_p->delete_children();
							calculate_node(cut_p);

							if (trace)
							{
								//						proofout << *r <<  " 0" << std::endl;
								std::cout << "c cut to:" << cut_p->to_string() << std::endl;
							}
						}
						else
						{
							calculate_node(p);
						}
					}
				}
				return result;
			}

			create_rule_result create_rule(clause* cl, bool trace)
			{
				rule st;
				create_rule_result result;
				result.created = false;
				result.empty_set = false;

				if (cl->is_switcher && cl->length > 1) return result;

				for (int i = 0; i < cl->length; i++)
				{
					auto& clguess = cl->literals[i];
					st.insert_or_assign(clguess.ln, literal_traits::get_anti_literal_value(clguess.l));
				}

				result = create_rule_impl(&root, st, trace);

				return result;
			}

			void claim(int _level, clause* _c)
			{
				auto& lc = literal_claims[_c->current_guess()->ln];
				if (lc.claimant && lc.claimant != _c) {
					std::cout << "c claim error " << std::endl;
				}
				lc.claimant = _c;
				int last_guess = lc.guess;
				lc.guess = _c->current_guess()->l;
				assignments.set_literal(lc.guess);
			}

			void assign(int _level, clause* _c)
			{
				auto& lc = literal_claims[_c->current_guess()->ln];
			}

			literal_claim& operator[](int _id)
			{
				return literal_claims[_id];
			}

			bool is_claimable(clause* cl, literal_selection* guess)
			{
				return cl->is_switcher;
			}

			literal_selection* get_assignment(clause* cl)
			{
				auto guess = cl->first_guess();
				clause_count++;
				while (guess)
				{
					if (can_assign(cl, guess))
						return guess;
					guess = cl->next_guess(guess);
				}
				return guess;
			}

			literal_selection* get_claim(clause* cl, bool trace = false)
			{
				auto guess = cl->current_guess();
				literal_selection* initial = nullptr;
#if DEBUG_CLAIM_CHECKING
				std::cout << "c get_claim start " << cl->to_string() << std::endl;
#endif
				clause_count++;
				if (guess) {
					initial = guess->anti_guess;
					if (initial && can_guess(cl, initial, trace))
						return initial;
				}

				guess = cl->first_guess();
				while (guess)
				{
#if DEBUG_CLAIM_CHECKING					
					std::cout << "c check " << guess->l << std::endl;
#endif
					if (guess != initial && can_guess(cl, guess, trace))
						return guess;
					guess = cl->next_guess(guess);

				}
				return guess;
			}

			void claim(clause* cl, literal_selection* new_guess, int level)
			{
				cl->select_guess(new_guess);
				claim(level, cl);
			}

			void set(clause* cl, literal_selection* new_guess, int level)
			{
				cl->select_guess(new_guess);
				assignments.set_literal(new_guess->l);
				auto& lc = literal_claims[new_guess->ln];
				lc.guess = new_guess->l;
			}

			literal_selection* assign(clause* cl, literal_selection* new_guess, int level)
			{
				cl->select_guess(new_guess);
				assignments.set_literal(new_guess->l);
				assign(level, cl);
				return new_guess;
			}

			bool analyze_guess(clause* cl, literal_selection* guess, std::vector<rule_node*>& violations, bool faux_proof)
			{
				clause_count++;
				literal_name ln = guess->ln;
				int sz = violations.size();
				find_matches(&root, &root, guess->l, violations, faux_proof);
				bool result = violations.size() == sz;
				return result;
			}

			bool can_guess(clause* cl, literal_selection* guess, bool faux_proof)
			{

				bool result = has_matches(&root, &root, guess->l);
				/*				if (faux_proof) {
									for (int i = sz; i < violations.size(); i++) {
										std::cout << "c violation:" << violations[i]->to_string() << std::endl;
									}
								} */
				return !result;
			}

			int resolve(bool trace, clause* cl, clause* reset_clause)
			{
				int old_last_changed = 0;

			try_again:

				bool make_rule = false;
				if (trace) {
					std::cout << "c resolve:" << cl->to_string() << std::endl;
				}

				statement new_rule;

				create_rule_result crr;

				crr = create_rule(cl, trace);

				if (crr.empty_set) { // indicates empty set
					return -1;
				}
				else if (!crr.created && !cl->is_switcher) {
					std::cout << "c backjump error " << cl->to_string() << " " << std::endl;
					exit(1); // abort on a back jump error.
				}

				int level = -1;
				clause* adjustee = nullptr;
				literal_selection* guess = nullptr;

				std::vector<rule_node*> violations;

				for (int i = cl->length - 1; i >= 0 && !guess; i--) {
					literal_selection* ls = &cl->literals[i];
					clause* scl = literal_claims[ls->ln].claimant;
					if (trace) std::cout << "c checking:" << scl->to_string() << " (" << ls->l << ")" << std::endl;
					auto temp_guess = scl->find_guess(ls->l);
					if (temp_guess)
					{
						if (analyze_guess(scl, temp_guess, violations, trace)) {
							if (trace) std::cout << "c possible:" << scl->to_string() << " " << ls->l << std::endl;
							adjustee = scl;
							guess = temp_guess;
						}
						else {
							if (trace) std::cout << "c conflicts:" << " " << ls->l << std::endl;
						}
					}
					else if (!temp_guess)
					{
						if (trace) std::cout << "c not possible:" << scl->to_string() << " " << ls->l << std::endl;
					}
				}

				if (adjustee)
				{
					claim(adjustee, guess, adjustee->level);
					if (trace) std::cout << "c claim:" << adjustee->to_string() << " " << guess->l << std::endl;
					level = literal_claims[cl->literals[0].ln].claimant->level - 1;
					if (level < 0) {
						level = 0;
					}
				}
				else if (violations.size() > 0)
				{
					cdcl_rule_count++;

					rule temp_rule;

					if (trace) {
						std::cout << std::endl << "c cdcl conflict analysis of :" << cl->to_string() << std::endl;
					}

					int smallest_violation_size = -1;
					rule_node* smallest_violation = nullptr;

					for (auto v : violations)
					{
						if (trace) {
							std::cout << "c conflict:" << v->to_string() << std::endl;
						}
						auto r = v;
						int violation_size = 0;
						while (r->parent)
						{
							if (r->ln != cl->first_guess()->ln) {
								temp_rule.insert_or_assign(r->ln, r->l);
							}
							r = r->parent;
							violation_size++;
						}
						if (smallest_violation_size == -1 || violation_size < smallest_violation_size)
						{
							smallest_violation_size = violation_size;
							smallest_violation = v;
						}
					}

					if (trace) std::cout << "c cdcl rule:" << temp_rule << std::endl;

					statement new_rule;
					for (auto rkv : temp_rule) {
						new_rule.add(rkv.second);
					}

					if (new_rule.size() == 0)
					{
						if (trace) std::cout << "c new_rule is empty set:" << new_rule << std::endl;
						level = -1;
					}
					else
					{
						auto rule_result = create_rule(new_rule, trace);
						if (rule_result.empty_set) {
							if (trace) std::cout << "c new_rule reduces to empty set:" << new_rule << std::endl;
							level = -1;
						}
						else {
							literal l = literal_traits::null_literal;
							literal_name must_jump_literal = literal_traits::get_literal_name(new_rule[0]);

							literal new_guess_literal = this->get_backjump_guess(l);
							literal_name ngln = literal_traits::get_literal_name(new_guess_literal);
							while (new_guess_literal && ngln > must_jump_literal)
							{
								new_guess_literal = this->get_backjump_guess(new_guess_literal);
								ngln = literal_traits::get_literal_name(new_guess_literal);
							}

							literal_name jump_literal = literal_traits::null_literal;
							if (ngln && ngln < must_jump_literal)
							{
								if (trace) std::cout << "c using backjump calculation " << ngln << " as backjump to force rule." << std::endl;
								jump_literal = ngln;
							}
							else
							{
								if (trace) std::cout << "c start of rule " << must_jump_literal << " as backjump to force rule." << std::endl;
								jump_literal = must_jump_literal;
							}

							auto lc = literal_claims[jump_literal];
							level = lc.claimant->level;
							guess = get_claim(lc.claimant, trace);
							if (guess) {
								lc.claimant->select_guess(guess);
								claim(level, lc.claimant);
								return level;
							}
							else {
								if (trace) std::cout << "c can't claim, retrying:" << lc.claimant->to_string() << std::endl;
								cl = lc.claimant;
								goto try_again;
							}

							if (trace) std::cout << "c can't find a guess:" << new_rule << std::endl;
							level = -1;
						}
					}
				}

				return level;
			}

			inline bool can_assign(clause* cl, literal_selection* guess)
			{
				bool c = literal_claims[guess->ln].guess == guess->l;
				return c;
			}

			static void test()
			{

				jcollection test_base2;
				test_base2.resize(4);

				statement r1 = parse_dimacs("-1 2 -3", false);
				test_base2.create_rule(r1, true);
				statement r2 = parse_dimacs("-1 -2 -3", false);
				test_base2.create_rule(r2, true);

				jcollection test_base;

				test_base.resize(10);

				// base statement
				statement base_rule = parse_dimacs("1 2 3 4 5", false);

				// exists cases
				statement existstrue1 = parse_dimacs("1 2 3 4 5", false);
				statement existstrue2 = parse_dimacs("1 2 3 4 5 6 7", false);

				statement notexists1 = parse_dimacs("2 3 4", false);
				statement notexists2 = parse_dimacs("1 3 5", false);

				test_base.create_rule(base_rule, true);

				if (!test_base.exists(existstrue1)) {
					std::cout << "exists fail 1" << std::endl;
				}
				if (!test_base.exists(existstrue2)) {
					std::cout << "exists fail 2" << std::endl;
				}
				if (test_base.exists(notexists1)) {
					std::cout << "exists fail 3" << std::endl;
				}
				if (test_base.exists(notexists2)) {
					std::cout << "exists fail 4" << std::endl;
				}

				// reductions, invalid
				statement invalid_reduction1 = parse_dimacs("1 2 3 -4 -5", false);
				statement invalid_reduction2 = parse_dimacs("1 2 3 -4 7", false);
				statement invalid_reduction3 = parse_dimacs("1 2 3 4 -6 7", false);
				statement invalid_reduction4 = parse_dimacs("3 4 -6 7", false);

				literal r = literal_traits::null_literal;
				r = test_base.reduces(invalid_reduction1);
				if (r) {
					std::cout << "fails reduction 1" << std::endl;
				}
				r = test_base.reduces(invalid_reduction2);
				if (r) {
					std::cout << "fails reduction 2" << std::endl;
				}
				r = test_base.reduces(invalid_reduction3);
				if (r) {
					std::cout << "fails reduction 3" << std::endl;
				}
				r = test_base.reduces(invalid_reduction4);
				if (r) {
					std::cout << "fails reduction 4" << std::endl;
				}

				// reductions, valid
				statement valid_reduction1 = parse_dimacs("-1 2 3 4 5", false);
				statement valid_reduction2 = parse_dimacs("1 2 3 -4 5", false);
				statement valid_reduction3 = parse_dimacs("1 2 3 4 -5", false);

				r = test_base.reduces(valid_reduction1);
				if (!r || r != -1) {
					std::cout << "fails reduction 5" << std::endl;
				}

				r = test_base.reduces(valid_reduction2);
				if (!r || r != -4) {
					std::cout << "fails reduction 6" << std::endl;
				}

				r = test_base.reduces(valid_reduction3);
				if (!r || r != -5) {
					std::cout << "fails reduction 7" << std::endl;
				}

				statement reduction_rule1 = parse_dimacs("1 2 -3 4 5", false);
				test_base.create_rule(reduction_rule1, true);

				statement reduction_rule2 = parse_dimacs("1 2 3 4 -5", false);
				test_base.create_rule(reduction_rule2, true);

				statement redexists1 = parse_dimacs("1 2 4 5", false);
				if (!test_base.exists(redexists1)) {
					std::cout << "redexists fail 1" << std::endl;
				}
				statement redexists2 = parse_dimacs("1 2 3 4", false);
				if (!test_base.exists(redexists2)) {
					std::cout << "redexists fail 2" << std::endl;
				}
				if (!test_base.exists(base_rule)) {
					std::cout << "redexists fail 3" << std::endl;
				}

				statement temp = parse_dimacs(
					"-1 0\n"
					"-2 2 0\n"
					"-3 3 0\n"
					"1 -2 -3 0\n"
					"1 -2 3 0\n"
					"1 2 -3 0\n"
					"1 2 3 0\n"
				);

				statement removed_literals;
				watch_list c = temp.get_clauses(removed_literals, false);

				int lvl = 0;
				for (auto& cl : c)
				{
					cl->level = lvl++;
				}

				jcollection tb2;
				tb2.resize(5);

				literal_selection* g = nullptr;
				int level = 0;

			tryitout:

				std::cout << "---------------" << std::endl;

				if (level < 0)
					goto unsat;

				switch (level) {
				case 0:
					g = tb2.get_claim(c[0], true);
					if (g) {
						c[0]->select_guess(g);
						tb2.claim(0, c[0]);
					}
					std::cout << std::endl << c[0]->to_string() << std::endl;
					[[fallthrough]];
				case 1:
					g = tb2.get_claim(c[1], true);
					if (g) {
						c[1]->select_guess(g);
						tb2.claim(1, c[1]);
					}
					std::cout << std::endl << c[1]->to_string() << std::endl;
					[[fallthrough]];
				case 2:
					g = tb2.get_claim(c[2], true);
					if (g)
					{
						c[2]->select_guess(g);
						tb2.claim(1, c[2]);
					}
					std::cout << std::endl << c[2]->to_string() << std::endl;
					[[fallthrough]];
				case 3:
					g = tb2.get_assignment(c[3]);
					if (g) {
						c[3]->select_guess(g);
						tb2.assign(1, c[3]);
					}
					else
					{
						if (level = tb2.resolve(true, c[3], c[0]))
							goto tryitout;
						else
							goto unsat;
					}
					std::cout << std::endl << c[3]->to_string() << std::endl;
					[[fallthrough]];
				case 4:
					g = tb2.get_assignment(c[4]);
					if (g) {
						c[4]->select_guess(g);
						tb2.assign(1, c[4]);
					}
					else
					{
						if (level = tb2.resolve(true, c[4], c[0]))
							goto tryitout;
						else
							goto unsat;
					}
					std::cout << std::endl << c[4]->to_string() << std::endl;
					[[fallthrough]];
				case 5:
					g = tb2.get_assignment(c[5]);
					if (g) {
						c[5]->select_guess(g);
						tb2.assign(1, c[5]);
					}
					else
					{
						if (level = tb2.resolve(true, c[5], c[0]))
							goto tryitout;
						else
							goto unsat;
					}
					std::cout << std::endl << c[5]->to_string() << std::endl;
					[[fallthrough]];
				case 6:
					g = tb2.get_assignment(c[6]);
					if (g) {
						c[6]->select_guess(g);
						tb2.assign(1, c[6]);
					}
					else
					{
						if (level = tb2.resolve(true, c[6], c[0]))
							goto tryitout;
						else
							goto unsat;
					}
					std::cout << std::endl << c[6]->to_string() << std::endl;
				}

			sat:
				std::cout << "Test fails, should be unsatisfiable." << std::endl;
				goto endofit;

			unsat:
				std::cout << "Test passes, is unsat." << std::endl;

			endofit:
				std::cout << "end of tests." << std::endl;

				/*
  -1 0
  1    -2  -3  0
  1    -2  3  0
  1    2  -3  0
1    2  3  0

				*/

			}


		};

	}
};

