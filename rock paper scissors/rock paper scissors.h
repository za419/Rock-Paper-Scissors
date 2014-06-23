// Rock paper scissors.h: Defines the backend for rock paper scissors functionality.

#ifndef __RPS_BACKEND_ENGINE
#define __RPS_BACKEND_ENGINE
#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <ctime>
#include <map>
#include <algorithm>
#include <string>
#define __NOSTACKREP
#include "errors.h"
#undef __NOSTACKREP

enum choice
{
	rock,
	paper,
	scissors,
	nochoice // Used to signify a processor round in which no gameplay takes place, ie a statistics print, history print, etc, to enable this to be entered cleanly in the plchoice helper, or an error.
};

enum result
{
	yes,
	no,
	tie,
	noround, // Used to signify to the caller function that no gameplay took place, and hence, no result occured. In this case, API records are not updated, and the system should not display a result.
	error // Used to signify an error in choice processing, and that the specified round has been undone.
};

enum respos // Result position
{
	left,
	right,
	tied,
	fail // Returned in case of comparing nochoice, or another impossible comparison
};

enum rescode // Result code
{
	success, // Succeeded
	failure // Failed
};

namespace backend
{
	using std::vector;
	using std::map;
	using std::pair;
	using std::make_pair;
	namespace data
	{
		vector<const choice> myhistory;
		vector<const choice> plhistory;
		vector<const result> cpuwonhist;
		map<pair<const choice, const choice>, choice> lookup; // The first member of the pair is the computer's choice, the second the players pick, and the other is the players choice the next round.
		bool lkupmode (false);
		char* username (nullptr);
		std::fstream database;
	}
	const unsigned docvers (0);
	const choice randchoice () throw ()
	{
		srand ((unsigned)time(NULL));
		return choice(rand()%(int(scissors)+1));
	}
	const choice nomem_getchoice () throw () // Does not go back into the records to make an algorithm.
	{
		using namespace data;
		if (myhistory.empty())
		{
			choice out (randchoice());
			myhistory.push_back(out);
			return out;
		}
		else
		{
			if (cpuwonhist.back()==yes)
			{
				if (myhistory.back()==rock)
				{
					if (rand()%2)
					{
						myhistory.push_back (scissors);
						return scissors;
					}
					myhistory.push_back(paper);
					return paper;
				}
				else if (myhistory.back()==paper)
				{
					if (rand()%2)
					{
						myhistory.push_back (rock);
						return rock;
					}
					myhistory.push_back(scissors);
					return scissors;
				}
				else if (myhistory.back()==scissors)
				{
					if (rand()%2)
					{
						myhistory.push_back (paper);
						return paper;
					}
					myhistory.push_back(rock);
					return rock;
				}
				else
					return nochoice;
			}
			else if (cpuwonhist.back()==no)
			{
				if (myhistory.back()==rock)
				{
					if (rand()%2)
					{
						myhistory.push_back (paper);
						return paper;
					}
					myhistory.push_back(rock);
					return rock;
				}
				else if (myhistory.back()==paper)
				{
					if (rand()%2)
					{
						myhistory.push_back (scissors);
						return scissors;
					}
					myhistory.push_back(paper);
					return paper;
				}
				else if (myhistory.back()==scissors)
				{
					if (rand()%2)
					{
						myhistory.push_back (rock);
						return rock;
					}
					myhistory.push_back(scissors);
					return scissors;
				}
				else
					return nochoice;
			}
			else if (cpuwonhist.back()==tie)
			{
				choice out (randchoice());
				myhistory.push_back(out);
				return out;
			}
			else
				return nochoice;
		}
	}
	const choice sessmem_getchoice () throw ()
	{
		using namespace data;
		if (myhistory.empty())
		{
			choice out (randchoice());
			myhistory.push_back(out);
			return out;
		}
		else
		{
			switch (lookup[make_pair(myhistory.back(),plhistory.back())])
			{
			case rock:
				myhistory.push_back (paper);
				return paper;
			case paper:
				myhistory.push_back (scissors);
				return scissors;
			case scissors:
				myhistory.push_back (rock);
				return rock;
			default:
				return nochoice;
			}
		}
	}
	void fillsessmem () throw ()
	{
		using namespace data;
		lookup[make_pair(rock,rock)]=randchoice();
		lookup[make_pair(rock,paper)]=randchoice();
		lookup[make_pair(rock,scissors)]=randchoice();
		lookup[make_pair(paper,rock)]=randchoice();
		lookup[make_pair(paper,paper)]=randchoice();
		lookup[make_pair(paper,scissors)]=randchoice();
		lookup[make_pair(scissors,rock)]=randchoice();
		lookup[make_pair(scissors,paper)]=randchoice();
		lookup[make_pair(scissors,scissors)]=randchoice();
	}
	const choice (*getchoice) () (nomem_getchoice); // Set this to the appropiate computer selection function. Defaults to nomem_getchoice.
	const choice plchoice (); // When defining this, the function should NOT (!) add the result to the player history (data::plhistory) vector, as the API itself does that.
	const respos won (const choice& lhs, const choice& rhs) throw ()
	{
		if (lhs==nochoice || rhs==nochoice)
			return fail;
		switch (lhs)
		{
		case rock:
			switch (rhs)
			{
			case rock:
				return tied;
			case paper:
				return right;
			case scissors:
				return left;
			default:
				return fail;
			}
			break;
		case paper:
			switch (rhs)
			{
			case rock:
				return left;
			case paper:
				return tied;
			case scissors:
				return right;
			default:
				return fail;
			}
			break;
		case scissors:
			switch (rhs)
			{
			case rock:
				return right;
			case paper:
				return left;
			case scissors:
				return tied;
			default:
				return fail;
			}
			break;
		default:
			return fail;
		}
	}
	const result playround () throw () // Returns if the computer won.
	{
		choice myold,yourold;
		if (data::lkupmode)
		{
			if (data::myhistory.empty())
				fillsessmem();
			else
			{
				myold=data::myhistory.back();
				yourold=data::plhistory.back();
			}
		}
		const choice mine=getchoice();
		if (mine==nochoice) // Error.
			return error;
		const choice yours=plchoice();
		if (yours==nochoice) // Processor round.
		{
			data::myhistory.pop_back();
			return noround;
		}
		if (data::lkupmode && !data::cpuwonhist.empty())
			data::lookup[make_pair (myold, yourold)]=yours;
		data::plhistory.push_back (yours);
		switch (mine)
		{
		case rock:
			switch (yours)
			{
			case rock:
				data::cpuwonhist.push_back(tie);
				return tie;
			case paper:
				data::cpuwonhist.push_back(no);
				return no;
			case scissors:
				data::cpuwonhist.push_back(yes);
				return yes;
			default:
				data::myhistory.pop_back();
				data::plhistory.pop_back();
				return error;
			}
			break;
		case paper:
			switch (yours)
			{
			case rock:
				data::cpuwonhist.push_back(yes);
				return yes;
			case paper:
				data::cpuwonhist.push_back(tie);
				return tie;
			case scissors:
				data::cpuwonhist.push_back(no);
				return no;
			default:
				data::myhistory.pop_back();
				data::plhistory.pop_back();
				return error;
			}
			break;
		case scissors:
			switch (yours)
			{
			case rock:
				data::cpuwonhist.push_back(no);
				return no;
			case paper:
				data::cpuwonhist.push_back(yes);
				return yes;
			case scissors:
				data::cpuwonhist.push_back(tie);
				return tie;
			default:
				data::myhistory.pop_back();
				data::plhistory.pop_back();
				return error;
			}
			break;
		default:
			return error;
		}
	}
	std::ostream& disphistto (std::ostream& stream)
	{
		using namespace data;
		const size_t rounds (cpuwonhist.size());
		unsigned long won (0), lost (0), tied (0);
		for (size_t i=0; i<rounds; i++)
		{
			switch (cpuwonhist[i])
			{
			case yes:
				++won;
				break;
			case no:
				++lost;
				break;
			case tie:
				++tied;
				break;
			default:
				stream<<"Error in counting win/lose/tie history.\n";
				return stream;
			}
		}
		stream<<"Total rounds played: "<<rounds<<".\n";
		stream.setf(0,std::ios::floatfield);
		stream.precision((std::streamsize)4);
		if (rounds)
		{
			stream<<"Computer victories: "<<won<<", "<<(double(won)/double(rounds))*100<<"%.\n";
			stream<<(data::username ? data::username : "Player")<<" victories: "<<lost<<", "<<(double(lost)/double(rounds))*100<<"%.\n";
			stream<<"Ties: "<<tied<<", "<<(double(tied)/double(rounds))*100<<"%.\n\n";
		}
		else // Ensures correct results in case of no results.
		{
			stream<<"Computer victories: 0, 0%.\n";
			if (data::username)
				stream<<data::username<<" victories: 0, 0%.\n";
			else
				stream<<"Player victories: 0, 0%.\n";
			stream<<"Ties: 0, 0%.\n";
		}
		return stream;
	}
	const rescode history (std::ostream& stream) throw ()
	{
		for (size_t i=0; i<data::cpuwonhist.size(); i++)
		{
			stream<<"Round "<<i+1<<": ";
			switch (data::cpuwonhist[i])
			{
			case yes:
				stream<<"computer won.\n";
				break;
			case no:
				if (data::username)
					stream<<data::username<<" won.\n";
				else
					stream<<"player won.\n";
				break;
			case tie:
				stream<<"tied.\n";
				break;
			default:
				stream<<"ERROR: bad result code. Exiting history handler...\n";
				return failure;
			}
		}
		return success;
	}
	const rescode comphist (std::ostream& stream) throw ()
	{
		try
		{
			for (size_t i=0; i<data::cpuwonhist.size(); i++)
			{
				stream<<"Round "<<i+1<<":\n    Computer choice: ";
				switch (data::myhistory.at(i))
				{
				case rock:
					if (data::username)
						stream<<"rock.\n    "<<data::username<<" choice: ";
					else
						stream<<"rock.\n    Player choice: ";
					break;
				case paper:
					if (data::username)
						stream<<"paper.\n    "<<data::username<<" choice: ";
					else
						stream<<"paper.\n    Player choice: ";
					break;
				case scissors:
					if (data::username)
						stream<<"scissors.\n    "<<data::username<<" choice: ";
					else
						stream<<"scissors.\n    Player choice: ";
					break;
				default:
					stream<<"ERROR: bad choice code. Exiting history handler...\n";
					return failure;
				}
				switch (data::plhistory.at(i))
				{
				case rock:
					stream<<"rock.\n    Result: ";
					break;
				case paper:
					stream<<"paper.\n    Result: ";
					break;
				case scissors:
					stream<<"scissors.\n    Result: ";
					break;
				default:
					stream<<"ERROR: bad choice code. Exiting history handler...\n";
					return failure;
				}
				switch (data::cpuwonhist[i])
				{
				case yes:
					stream<<"computer wins.\n\n";
					break;
				case no:
					if (data::username)
						stream<<data::username<<" wins.\n\n";
					else
						stream<<"player wins.\n\n";
					break;
				case tie:
					stream<<"tie.\n\n";
					break;
				default:
					stream<<"ERROR: bad result code. Exiting history handler...\n";
					return failure;
				}
			}
		}
		catch (...)
		{
			stream<<"ERROR: the FDA, IRS, or any other government or non-government agency has not approved our accounting. (no, we are not affiliated with Enron...). Exiting history handler...\n";
			return failure;
		}
		return success;
	}
	const rescode load () throw ()
	{
		using namespace data;
		database.seekg (std::ios::beg);
		unsigned vers;
		database>>vers;
		debug<<"Current document version number: "<<docvers<<'\n';
		debug<<"Read database version number: "<<vers<<'\n';
		if (vers!=docvers)
			return failure;
		int curr;
		size_t num;
		database>>num; // Computer history load.
		myhistory.clear();
		debug<<"Read myhistory size: "<<num<<'\n';
		for (; num; --num)
		{
			database>>curr;
			myhistory.push_back ((choice)curr);
		}
		database>>num; // Player history load.
		plhistory.clear();
		debug<<"Read plhistory size: "<<num<<'\n';
		for (; num; --num)
		{
			database>>curr;
			plhistory.push_back ((choice)curr);
		}
		database>>num; // Win/loss history load.
		cpuwonhist.clear();
		debug<<"Read cpuwonhist size: "<<num<<'\n';
		for (; num; --num)
		{
			database>>curr;
			cpuwonhist.push_back ((result)curr);
		}
		// Lookup load.
		database>>curr;
		lookup[make_pair (rock, rock)]=(choice)curr;
		database>>curr;
		lookup[make_pair (rock, paper)]=(choice)curr;
		database>>curr;
		lookup[make_pair (rock, scissors)]=(choice)curr;
		database>>curr;
		lookup[make_pair (paper, rock)]=(choice)curr;
		database>>curr;
		lookup[make_pair (paper, paper)]=(choice)curr;
		database>>curr;
		lookup[make_pair (paper, scissors)]=(choice)curr;
		database>>curr;
		lookup[make_pair (scissors, rock)]=(choice)curr;
		database>>curr;
		lookup[make_pair (scissors, paper)]=(choice)curr;
		database>>curr;
		lookup[make_pair (scissors, scissors)]=(choice)curr;
		return success;
	}
	const rescode compatload () throw ()
	{
		using namespace data;
		unsigned vers;
		database>>vers;
		switch (vers)
		{
		case 0:
			int curr;
			size_t num;
			database>>num; // Computer history load.
			myhistory.clear();
			for (; num; --num)
			{
				database>>curr;
				myhistory.push_back ((choice)curr);
			}
			database>>num; // Player history load.
			plhistory.clear();
			for (; num; --num)
			{
				database>>curr;
				myhistory.push_back ((choice)curr);
			}
			database>>num; // Win/loss history load.
			cpuwonhist.clear();
			for (; num; --num)
			{
				database>>curr;
				cpuwonhist.push_back ((result)curr);
			}
			// Lookup load.
			database>>curr;
			lookup[make_pair (rock, rock)]=(choice)curr;
			database>>curr;
			lookup[make_pair (rock, paper)]=(choice)curr;
			database>>curr;
			lookup[make_pair (rock, scissors)]=(choice)curr;
			database>>curr;
			lookup[make_pair (paper, rock)]=(choice)curr;
			database>>curr;
			lookup[make_pair (paper, paper)]=(choice)curr;
			database>>curr;
			lookup[make_pair (paper, scissors)]=(choice)curr;
			database>>curr;
			lookup[make_pair (scissors, rock)]=(choice)curr;
			database>>curr;
			lookup[make_pair (scissors, paper)]=(choice)curr;
			database>>curr;
			lookup[make_pair (scissors, scissors)]=(choice)curr;
			return success;
		/*case docvers: // Uncomment this label when the current document version is not already in the switch.
			return load();*/
		default:
			return failure;
		}
	}
	void save () throw ()
	{
		using namespace data;
		database.close();
		database.open((std::string(username)+".db").c_str(),std::ios::in|std::ios::out|std::ios::trunc); // Reopen to clear the file.
		database.seekp(std::ios::beg);
		debug_print ("Current document version number: ")
		debug_print (docvers)
		debug_print ('\n')
		debug_print ("Saving document version number: ")
		debug_print (docvers)
		debug_print ('\n')
		database<<docvers<<'\n'<<(size_t)myhistory.size()<<'\n';
		for (size_t i=0; i<myhistory.size(); i++)
			database<<(int)myhistory[i]<<'\n';
		database<<(size_t)plhistory.size()<<'\n';
		for (size_t i=0; i<plhistory.size(); i++)
			database<<(int)plhistory[i]<<'\n';
		database<<(size_t)cpuwonhist.size()<<'\n';
		for (size_t i=0; i<cpuwonhist.size(); i++)
			database<<(int)cpuwonhist[i]<<'\n';
		database<<(int)lookup[make_pair(rock,rock)]<<'\n';
		database<<(int)lookup[make_pair(rock,paper)]<<'\n';
		database<<(int)lookup[make_pair(rock,scissors)]<<'\n';
		database<<(int)lookup[make_pair(paper,rock)]<<'\n';
		database<<(int)lookup[make_pair(paper,paper)]<<'\n';
		database<<(int)lookup[make_pair(paper,scissors)]<<'\n';
		database<<(int)lookup[make_pair(scissors,rock)]<<'\n';
		database<<(int)lookup[make_pair(scissors,paper)]<<'\n';
		database<<(int)lookup[make_pair(scissors,scissors)]<<'\n';
	}
}
#endif