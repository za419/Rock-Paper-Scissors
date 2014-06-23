// rock paper scissors.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <algorithm>
#include "rock paper scissors.h"
#define stricmp _stricmp
using std::cout;
using std::cin;
using std::ifstream;
using std::ofstream;
using std::string;
using std::ios;

namespace backend
{
	const choice plchoice () throw ()
	{
		char ans[101];
		while (true)
		{
			cout<<"Enter rock, paper, or scissors to play a round, statistics for win/loss/tie\nstatistics, history for rounds history, comphist for complex rounds history,\nclear to clear records and start again, or exit to exit:\n";
			cin>>ans;
			if (!stricmp(ans,"rock"))
				return rock;
			else if (!stricmp(ans,"paper"))
				return paper;
			else if (!stricmp(ans,"scissors"))
				return scissors;
			else if (!stricmp(ans,"statistics"))
			{
				disphistto(cout<<"\n    Statistics:\n");
				return nochoice;
			}
			else if (!stricmp(ans,"history"))
			{
				history(cout<<"\n    Round history:\n\n");
				cout<<"\nDone\n\n";
				return nochoice;
			}
			else if (!stricmp(ans,"comphist"))
			{
				comphist(cout<<"\n    Complex round history:\n\n");
				cout<<"\nDone.\n\n";
				return nochoice;
			}
			else if (!stricmp(ans,"clear"))
			{
				cout<<"Ready to clear all history and restart the game... Are you sure you would like to do this?? (y/N)\n";
				char res;
				scanf ("%c", &res);
				res='a';
				if (scanf ("%c",&res)==0 || res=='\n' || toupper(res)=='N')
				{
					cout<<"Undoing...Done.\n\n";
					return nochoice;
				}
				cout<<"OK...Erasing records...\n\n";
				data::cpuwonhist.clear();
				data::myhistory.clear();
				data::myhistory.push_back (rock); // This is just here in order to avoid a 'vector empty before pop' caused by the way the system handles a processor round.
				data::plhistory.clear();
				return nochoice;
			}
			else if (!stricmp(ans,"exit"))
			{
				cout<<"Now exiting...\nGoodbye!";
				if (data::username)
				{
					save();
					data::database.close();
					delete[] data::username;
				}
				exit(0);
			}
			else
				cout<<"Invalid response: Try again!\n";
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc>1)
	{
		if (!stricmp (argv[1], "session"))
		{
			backend::data::lkupmode=true;
			backend::getchoice=backend::sessmem_getchoice;
		}
		else if (!stricmp (argv[1], "database"))
		{
			backend::data::lkupmode=true;
			backend::getchoice=backend::sessmem_getchoice;
			if (argc>2)
			{
				backend::data::username=new char [strlen (argv[2])+1];
				strcpy (backend::data::username, argv[2]);
			}
			else
			{
				cout<<"Usage: "<<argv[0]<<" memorymode [username]\n";
				cout<<"  memorymode can be any of:\n";
				cout<<"    none (uses a one-move algorithm)\n";
				cout<<"    session (uses session memory for the algorithm)\n";
				cout<<"    database (uses and records user-specific data for the algorithm)\n";
				cout<<"Username is only used for database mode, and allows all prior data to be displayed.\n";
				cout<<"Please enter your username.\n";
				std::string user;
				std::getline (cin,user);
				backend::data::username=new char [user.size()+1];
				strcpy (backend::data::username, user.c_str());
			}
			const bool load ((bool)ifstream ((string(backend::data::username)+".db").c_str()));
			backend::data::database.open ((string(backend::data::username)+".db").c_str(),ios::in|ios::out);
			if (load)
			{
				if (backend::load ()==failure)
				{
					cout<<"The system shows that your database file exists, but is not in the current format.\nAttempting load of prior database formats...\n";
					if (backend::compatload ()==failure)
					{
						cout<<"The compatibility mode loader was not able to open your database.\n";
						char yn;
						while (1)
						{
							cout << "Would you like to wipe the old database and reopen it (Y/N)?\n";
							if (toupper (yn)=='Y')
							{
								cout<<"Ok...Wiping old database and starting anew...\n";
								backend::data::database.close();
								backend::data::database.open ((string(backend::data::username)+".db").c_str(), ios::in|ios::out|ios::trunc);
								break;
							}
							if (toupper (yn)=='N')
							{
								while (1)
								{
									cout<<"Ok...Would you like to backup the old database and make a new one (Y/N)?\n";
									if (toupper (yn)=='Y')
									{
										cout<<"Backing up...\n";
										backend::data::database.seekp (0, ios::end);
										std::streamsize filsize (backend::data::database.tellp ());
										char* old (new char [size_t(filsize)]);
										backend::data::database.read (old, filsize);
										ofstream _new ((string(backend::data::username)+".DB.BAK").c_str(), ios::out);
										_new.write (old, filsize);
										_new.close();
										cout<<"Done.\nBackup next to the program executable, as ",backend::data::username,".DB.BAK.\n";
										delete[] old;
										break;
									}
									if (toupper (yn)=='N')
									{
										cout<<"Ok...switching to session memory mode...\n";
										delete[] backend::data::username;
											backend::data::username=nullptr;
										backend::data::database.close();
										break;
									}
								}
								break;
							}
						}
					}
				}
			}
		}
		else if (!stricmp (argv[1], "none")); // The program starts preconfigured for this mode. Take no action.	
		else
		{
			cout<<"Usage: "<<argv[0]<<" memorymode [username]\n";
			cout<<"  memorymode can be any of:\n";
			cout<<"    none (uses a one-move algorithm)\n";
			cout<<"    session (uses session memory for the algorithm)\n";
			cout<<"    database (uses and records user-specific data for the algorithm)\n";
			cout<<"Username is only used for database mode, and allows all prior data to be displayed.\n\n";
			while (1)
			{
				cout<<"Please enter your memorymode.\n";
				string mode;
				getline (cin, mode);
				if (!stricmp(mode.c_str(), "database"))
				{
					backend::data::lkupmode=true;
					backend::getchoice=backend::sessmem_getchoice;
					cout<<"Please enter your username.\n";
					std::string user;
					std::getline (cin,user);
					backend::data::username=new char [user.size()+1];
					strcpy (backend::data::username, user.c_str());
					const bool load ((bool)ifstream (backend::data::username));
					backend::data::database.open (backend::data::username);
					if (load)
					{
						if (backend::load ()==failure)
						{
							cout<<"The system shows that your database file exists, but is not in the current format.\nAttempting load of prior database formats...\n";
							if (backend::compatload ()==failure)
							{
								cout<<"The compatibility mode loader was not able to open your database.\n";
								char yn;
								while (1)
								{
									cout<<"Would you like to wipe your corrupted database to reopen it (Y/N)?\n";
									if (toupper (yn)=='Y')
									{
										cout<<"Ok...Wiping old database and starting anew...\n";
										backend::data::database.close();
										backend::data::database.open ((string(backend::data::username)+".db").c_str(), ios::in|ios::out|ios::trunc);
										break;
									}
									if (toupper (yn)=='N')
									{
										while (1)
										{
											cout<<"Ok...Would you like to backup the old database and make a new one (Y/N)?\n";
											if (toupper (yn)=='Y')
											{
												cout<<"Backing up...\n";
												backend::data::database.seekp (0, ios::end);
												std::streamsize filsize (backend::data::database.tellp ());
												char* old (new char [size_t(filsize)]);
												backend::data::database.read (old, filsize);
												ofstream _new ((string(backend::data::username)+".DB.BAK").c_str(), ios::out);
												_new.write (old, filsize);
												_new.close();
												cout<<"Backup next to the program executable, as ",backend::data::username,".DB.BAK.\n";
												delete[] old;
												break;
											}
											if (toupper (yn)=='N')
											{
												cout<<"Ok...switching to session memory mode...\n";
												delete[] backend::data::username;
												backend::data::username=nullptr;
												backend::data::database.close();
												break;
											}
										}
										break;
									}
								}
							}
						}
					}
					break;
				}
				else if (!stricmp (mode.c_str(), "session"))
				{
					backend::data::lkupmode=true;
					backend::getchoice=backend::sessmem_getchoice;
					break;
				}
				else if (!stricmp (mode.c_str(), "none"))
					break; // The program starts preconfigured for this mode, so no action needs to be taken, besides breaking the while loop.
				else
					cout<<"That is not a valid mode!\nTry again...\n";
			}
		}
	}
	else
	{
		cout<<"Usage: "<<argv[0]<<" memorymode [username]\n";
		cout<<"  memorymode can be any of:\n";
		cout<<"    none (uses a one-move algorithm)\n";
		cout<<"    session (uses session memory for the algorithm)\n";
		cout<<"    database (uses and records user-specific data for the algorithm)\n";
		cout<<"Username is only used for database mode, and allows all prior data to be displayed.\n\n";
		while (1)
		{
			cout<<"Please enter your memorymode.\n";
			string mode;
			getline (cin, mode);
			if (!stricmp(mode.c_str(), "database"))
			{
				backend::data::lkupmode=true;
				backend::getchoice=backend::sessmem_getchoice;
				cout<<"Please enter your username.\n";
				std::string user;
				std::getline (cin,user);
				backend::data::username=new char [user.size()+1];
				strcpy (backend::data::username, user.c_str());
				const bool load ((bool)ifstream ((string(backend::data::username)+".db").c_str()));
				backend::data::database.open ((string(backend::data::username)+".db").c_str(), ios::in|ios::out);
				if (load)
				{
					if (backend::load ()==failure)
					{
						cout<<"The system shows that your database file exists, but is not in the current format.\nAttempting load of prior database formats...\n";
						if (backend::compatload ()==failure)
						{
							cout<<"The compatibility mode loader was not able to open your database.\n";
							char yn;
							while (1)
							{
								cout<<"Would you like to wipe your corrupted database to reopen it (Y/N)?\n";
								if (toupper (yn)=='Y')
								{
									cout<<"Ok...Wiping old database and starting anew...\n";
									backend::data::database.close();
									backend::data::database.open ((string(backend::data::username)+".db").c_str(), ios::in|ios::out|ios::trunc);
									break;
								}
								if (toupper (yn)=='N')
								{
									while (1)
									{
										cout<<"Ok...Would you like to backup the old database and make a new one (Y/N)?";
										if (toupper (yn)=='Y')
										{
											cout<<"Alright...Doing it...\n";
											backend::data::database.seekp (0, ios::end);
											std::streamsize filsize (backend::data::database.tellp ());
											char* old (new char [size_t(filsize)]);
											backend::data::database.read (old, filsize);
											ofstream _new ((string(backend::data::username)+".DB.BAK").c_str(), ios::out);
											_new.write (old, filsize);
											_new.close();
											cout<<"Backup next to the program executable, as ",backend::data::username,".DB.BAK.\n";
											delete[] old;
											break;
										}
										if (toupper (yn)=='N')
										{
											cout<<"Ok...switching to session memory mode...\n";
											delete[] backend::data::username;
											backend::data::username=nullptr;
											backend::data::database.close();
											break;
										}
									}
									break;
								}
							}
						}
					}
				}
				break;
			}
			else if (!stricmp (mode.c_str(), "session"))
			{
				backend::data::lkupmode=true;
				backend::getchoice=backend::sessmem_getchoice;
				break;
			}
			else if (!stricmp (mode.c_str(), "none"))
				break; // The program starts preconfigured for this mode, so no action needs to be taken, besides breaking the while loop.
			else
				cout<<"That is not a valid mode!\nTry again...\n";
		}
	}
	if (!backend::data::username)
		cout<<"Welcome to your friendly console-based rock paper scissors program!\n";
	else
		cout<<"Welcome to your friendly console-based rock paper scissors program, "<<backend::data::username<<"!\n";
	while (1)
	{
		const result res (backend::playround());
		if (res==noround)
			continue;
		else if (res==error) // Ensure that the previous round data is not redisplayed.
		{
			cout<<"$#&! An internal error has occured. The record of the previous round has been deleted.\n";
			continue;
		}
		cout<<"I had: ";
		switch (backend::data::myhistory.back())
		{
		case rock:
			cout<<"rock.\n";
			break;
		case paper:
			cout<<"paper.\n";
			break;
		case scissors:
			cout<<"scissors.\n";
			break;
		default:
			cout<<"*&%&#&^@#$$@! Internal error in choice recovery! Aborting...\n";
			return 1;
		}
		cout<<"You had: ";
		switch (backend::data::plhistory.back())
		{
		case rock:
			cout<<"rock.\n";
			break;
		case paper:
			cout<<"paper.\n";
			break;
		case scissors:
			cout<<"scissors.\n";
			break;
		default:
			cout<<"*&%&#&^@#$$@! Internal error in choice recovery! Aborting...\n";
			return 1;
		}
		cout<<"That means that you ";
		switch (res)
		{
		case yes:
			cout<<"lost. Play again?\n";
			break;
		case no:
			cout<<"won. I challenge you to a rematch!\n";
			break;
		case tie:
			cout<<"tied with me. That means nothing, let's play again.\n";
			break;
		default:
			cout<<"&%&#&@#$$@! Internal error in result recovery! Aborting...\n";
			return 1;
		}
	}
	return 0;
}