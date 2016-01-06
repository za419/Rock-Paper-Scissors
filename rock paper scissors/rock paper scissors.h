// Rock paper scissors.h: Defines the backend for rock paper scissors functionality.
// TODO: Adjust session and database modes to make use of the lookback property

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
#include "debugio.h"
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
	failure, // Failed
	bad // Failed due to invalid request
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
		unsigned lookback(1); // How many pairs back to store and compare
		char* username (nullptr);
		std::string password;
		std::fstream database;
	}
	namespace hidden // Standard data hiding namespace.
	{
		namespace rockPaperScissors
		{
			struct randSeeder // Serves no function except to call srand() on construction. Main intent is to seed rand() on first init.
			{
				randSeeder() throw()
				{
					srand((unsigned)time(NULL));
				}
			} randSeedInit;
		}
	}
	const unsigned docvers(2);
	const unsigned MAGIC_NUMBER(0xBADC0DE);

	bool failOnBadHeader(unsigned versionCode) // Returns whether the document version standard specified by versionCode demands full load failure if a bad MAGIC_NUMBER is read.
	{
		switch (versionCode)
		{
		case 0: // Inital version.
			return false; // Document version 0 did not provide for any MAGIC_NUMBER. It doesn't matter what goes here.
		case 1: // First document format update: Add in the first MAGIC_NUMBER header. Note that no function checking is done in version 1.
			return false; // Document format 1 only has MAGIC_NUMBER as a basic check. There is no reason we should crash, its most likely a saver or loader bug.
		case 2: // Second document format update: Add in basic password support
#ifdef DEBUGGING // Since document format 2 was introduced as soon as MAGIC_NUMBER checking was completed, enable failure in debugging to detect regressions.
			return true; // Crash by default if debugging behavior is enabled
#else
			return false; // If debugging behavior is disabled, continue by default.
#endif
		default:
#ifdef DEBUGGING
			return true; // Crash by default if debugging behavior is enabled
#else
			return false; // If debugging behavior is disabled, continue by default.
#endif
		}
	}

	const choice randchoice() throw ()
	{
		return choice(rand() % (int(scissors) + 1));
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
		// Difficulty alteration based on win percentage
		unsigned long won(0);
		for (size_t i = 0; i < data::cpuwonhist.size(); ++i)
		{
			if (data::cpuwonhist[i]==yes)
				++won;
		}
		const double percent(won / (double)data::cpuwonhist.size());
		if (percent > .66) // This is too low. Lower the difficulty if it is under one.
		{
			debug_print("\nWarning: Player win percentage under 33%: "); 
			debug_print(1 - percent);
			debug_print(".\nReducing difficulty level (if possible)...");
			data::lookback = std::max(1u, data::lookback - unsigned((percent > .9 ? percent * 7 : 1))); // Limited to 1. We can't look back 0 rounds.
			debug_print("Done.\nNew difficulty level: ");
			debug_print(data::lookback);
			debug_print("\n\n");
		}
		else if (percent < .33)
		{
			debug_print("\nWarning: Player win percentage over 66%: ");
			debug_print(1 - percent);
			debug_print(".\nIncreasing difficulty level (if reasonable)...");
			data::lookback = std::min(1000u, data::lookback - unsigned((percent < .1 ? (1 - percent) * 7 : 1))); // Arbitrarily limited to 1000.
			debug_print("Done.\nNew difficulty level: ");
			debug_print(data::lookback);
			debug_print("\n\n");
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
	const rescode load() throw ()
	{
		using namespace data;
		database.seekg(std::ios::beg);
		unsigned header;
		database >> header;
		if (header != MAGIC_NUMBER)
		{
			debug_print("Failure on reading header number: Read ");
			debug_print(header);
			debug_print(", expected ");
			debug_print(MAGIC_NUMBER);
			debug_print('\n');
			if (failOnBadHeader(docvers))
			{
				debug << "Current document version, " << docvers << ", specifies for load() to not attempt to continue a document load after a bad header read.\n";
				return failure;
			}
			else
				debug << "Continuing as per document specifications.\n";
		}
		unsigned vers;
		database >> vers;
		debug << "Current document version number: " << docvers << '\n';
		debug << "Read database version number: " << vers << '\n';
		if (vers != docvers)
			return failure;
		std::string pass;
		database.clear();
		database.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::getline(database, pass);
		if (pass != password)
		{
			debug << "Invalid password: Was provided with " << password << ", read " << pass << ".\n";
			return bad;
		}
		database.clear();
		database.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		int curr;
		size_t num;
		database >> num; // Computer history load.
		myhistory.clear();
		debug << "Read myhistory size: " << num << '\n';
		for (; num; --num)
		{
			database >> curr;
			myhistory.push_back((choice)curr);
		}
		database >> num; // Player history load.
		plhistory.clear();
		debug << "Read plhistory size: " << num << '\n';
		for (; num; --num)
		{
			database >> curr;
			plhistory.push_back((choice)curr);
		}
		database >> num; // Win/loss history load.
		cpuwonhist.clear();
		debug << "Read cpuwonhist size: " << num << '\n';
		for (; num; --num)
		{
			database >> curr;
			cpuwonhist.push_back((result)curr);
		}
		// Lookup load.
		database >> curr;
		lookup[make_pair(rock, rock)] = (choice)curr;
		database >> curr;
		lookup[make_pair(rock, paper)] = (choice)curr;
		database >> curr;
		lookup[make_pair(rock, scissors)] = (choice)curr;
		database >> curr;
		lookup[make_pair(paper, rock)] = (choice)curr;
		database >> curr;
		lookup[make_pair(paper, paper)] = (choice)curr;
		database >> curr;
		lookup[make_pair(paper, scissors)] = (choice)curr;
		database >> curr;
		lookup[make_pair(scissors, rock)] = (choice)curr;
		database >> curr;
		lookup[make_pair(scissors, paper)] = (choice)curr;
		database >> curr;
		lookup[make_pair(scissors, scissors)] = (choice)curr;
		return success;
	}
	const rescode compatload() throw ()
	{
		using namespace data;
		database.seekg(std::ios::beg);
		unsigned header, vers;
		database >> header;
		if (header == 0) // If the first read is 0, then we are in document version 0, therefore we need to forgo the next read.
			vers = 0;
		else
			database >> vers;
		int curr;
		size_t num;
		switch (vers)
		{
		case 0: // Initial version loader.
			database >> num; // Computer history load.
			myhistory.clear();
			for (; num; --num)
			{
				database >> curr;
				myhistory.push_back((choice)curr);
			}
			database >> num; // Player history load.
			plhistory.clear();
			for (; num; --num)
			{
				database >> curr;
				myhistory.push_back((choice)curr);
			}
			database >> num; // Win/loss history load.
			cpuwonhist.clear();
			for (; num; --num)
			{
				database >> curr;
				cpuwonhist.push_back((result)curr);
			}
			// Lookup load.
			database >> curr;
			lookup[make_pair(rock, rock)] = (choice)curr;
			database >> curr;
			lookup[make_pair(rock, paper)] = (choice)curr;
			database >> curr;
			lookup[make_pair(rock, scissors)] = (choice)curr;
			database >> curr;
			lookup[make_pair(paper, rock)] = (choice)curr;
			database >> curr;
			lookup[make_pair(paper, paper)] = (choice)curr;
			database >> curr;
			lookup[make_pair(paper, scissors)] = (choice)curr;
			database >> curr;
			lookup[make_pair(scissors, rock)] = (choice)curr;
			database >> curr;
			lookup[make_pair(scissors, paper)] = (choice)curr;
			database >> curr;
			lookup[make_pair(scissors, scissors)] = (choice)curr;
			return success;
		case 1: // Loader code for the first version revision: Add in initial header support. Note that there is no support for endianness checking et al.
			if (header != 0xBADC0DE)
			{
				debug_print("Failure on reading header number: Read ");
				debug_print(header);
				debug_print(", expected ");
				debug_print(0xBADC0DE);
				if (failOnBadHeader(1))
				{
					debug << "Current document version, 1, specifies for load() to not attempt to continue a document load after a bad header read.\n";
					return failure;
				}
				else
					debug << "Continuing as per document specifications.\n";
			}
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
		/*case 2: // Loader code for the second document revision: Add in password support.
			if (header != 0xBADC0DE)
			{
				debug_print("Failure on reading header number: Read ");
				debug_print(header);
				debug_print(", expected ");
				debug_print(0xBADC0DE);
				if (failOnBadHeader(2))
				{
					debug << "Current document version, 2, specifies for load() to not attempt to continue a document load after a bad header read.\n";
					return failure;
				}
				else
					debug << "Continuing as per document specifications.\n";
			}
			std::string pass;
			std::getline(database, pass);
			if (pass != password)
			{
				debug << "Invalid password: Was provided with " << pass << ", read " << password << ".\n";
				return bad;
			}
			database >> num; // Computer history load.
			myhistory.clear();
			debug << "Read myhistory size: " << num << '\n';
			for (; num; --num)
			{
				database >> curr;
				myhistory.push_back((choice)curr);
			}
			database >> num; // Player history load.
			plhistory.clear();
			debug << "Read plhistory size: " << num << '\n';
			for (; num; --num)
			{
				database >> curr;
				plhistory.push_back((choice)curr);
			}
			database >> num; // Win/loss history load.
			cpuwonhist.clear();
			debug << "Read cpuwonhist size: " << num << '\n';
			for (; num; --num)
			{
				database >> curr;
				cpuwonhist.push_back((result)curr);
			}
			// Lookup load.
			database >> curr;
			lookup[make_pair(rock, rock)] = (choice)curr;
			database >> curr;
			lookup[make_pair(rock, paper)] = (choice)curr;
			database >> curr;
			lookup[make_pair(rock, scissors)] = (choice)curr;
			database >> curr;
			lookup[make_pair(paper, rock)] = (choice)curr;
			database >> curr;
			lookup[make_pair(paper, paper)] = (choice)curr;
			database >> curr;
			lookup[make_pair(paper, scissors)] = (choice)curr;
			database >> curr;
			lookup[make_pair(scissors, rock)] = (choice)curr;
			database >> curr;
			lookup[make_pair(scissors, paper)] = (choice)curr;
			database >> curr;
			lookup[make_pair(scissors, scissors)] = (choice)curr;
			return success;*/
		case docvers:
			return load();
		default:
			return failure;
		}
	}
	void save() throw ()
	{
		using namespace data;
		database.close();
		database.open((std::string(username) + ".db").c_str(), std::ios::in | std::ios::out | std::ios::trunc); // Reopen to clear the file.
		database.seekp(std::ios::beg);
		debug_print("Magic code: ");
		debug_print(MAGIC_NUMBER);
		debug_print('\n');
		debug_print("Current document version number: ");
		debug_print(docvers);
		debug_print('\n');
		debug_print("Saving document version number: ");
		debug_print(docvers);
		debug_print('\n');
		database << MAGIC_NUMBER << '\n' << docvers << '\n' << password << '\n' << (size_t)myhistory.size() << '\n';
		for (size_t i = 0; i<myhistory.size(); i++)
			database << (int)myhistory[i] << '\n';
		database << (size_t)plhistory.size() << '\n';
		for (size_t i = 0; i<plhistory.size(); i++)
			database << (int)plhistory[i] << '\n';
		database << (size_t)cpuwonhist.size() << '\n';
		for (size_t i = 0; i<cpuwonhist.size(); i++)
			database << (int)cpuwonhist[i] << '\n';
		database << (int)lookup[make_pair(rock, rock)] << '\n';
		database << (int)lookup[make_pair(rock, paper)] << '\n';
		database << (int)lookup[make_pair(rock, scissors)] << '\n';
		database << (int)lookup[make_pair(paper, rock)] << '\n';
		database << (int)lookup[make_pair(paper, paper)] << '\n';
		database << (int)lookup[make_pair(paper, scissors)] << '\n';
		database << (int)lookup[make_pair(scissors, rock)] << '\n';
		database << (int)lookup[make_pair(scissors, paper)] << '\n';
		database << (int)lookup[make_pair(scissors, scissors)] << '\n';
	}
}
#endif