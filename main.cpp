/* 
 *
 * Option trades runner
 *
 */

#include<iostream>
#include"option.h"
#include<string>
#include<vector>
#include<fstream>
#include<functional>
#include<unordered_map>
#include<map>

using namespace std; 

void readFirstDay(const vector<int>& index, const string& fileName, unordered_map<string, bool> &activeOption, unordered_map<string, vector<Option> > & optionInfo);
void readOtherDay(const vector<int>& index, const string& fileName, unordered_map<string, bool> &activeOption, unordered_map<string, vector<Option> > & optionInfo);
bool check(const vector<string>& vec, const vector<int>& index);

int main(int argc)
{
	bool hedge;
	string strategy;
	cout<<"please enter the trading strategy: hedge or naked"<<endl;
	cin>>strategy;

	if(strategy=="hedge")
		hedge=true;
	else if(strategy=="naked")
		hedge=false;
	else
	{
		cerr<<"error, invalid input"<<endl;
		exit(1);
	}


	// the key is the name of option and the element is the bool to record whether it is active in all trading days
	unordered_map<string, bool> activeOption; 
	// the key is the name of option, the element is the information of the option in each day as a vector, the index of the vector is the day. 
	// only the active records is contained in this data structure
	unordered_map<string, vector<Option> > optionInfo; 

	vector<int> index(13);
	index[0]=0;
	index[1]=4;
	index[2]=5;
	index[3]=10;
	index[4]=1;
	index[5]=2;
	index[6]=11;
	index[7]=16;
	index[8]=17;
	index[9]=26;
	index[10]=27;
	index[11]=28;
	index[12]=24;

	//"begin to read the record of the first day"
	readFirstDay(index, "20121004_option_tick_processed.tsv", activeOption, optionInfo);

	//read the record of the following days
	readOtherDay(index, "20121005_option_tick_processed.tsv", activeOption, optionInfo);
	readOtherDay(index, "20121008_option_tick_processed.tsv", activeOption, optionInfo);
	readOtherDay(index, "20121009_option_tick_processed.tsv", activeOption, optionInfo);
	readOtherDay(index, "20121010_option_tick_processed.tsv", activeOption, optionInfo);
	readOtherDay(index, "20121011_option_tick_processed.tsv", activeOption, optionInfo);
	readOtherDay(index, "20121012_option_tick_processed.tsv", activeOption, optionInfo);
	readOtherDay(index, "20121016_option_tick_processed.tsv", activeOption, optionInfo);
	readOtherDay(index, "20121017_option_tick_processed.tsv", activeOption, optionInfo);
	readOtherDay(index, "20121018_option_tick_processed.tsv", activeOption, optionInfo);
	readOtherDay(index, "20121019_option_tick_processed.tsv", activeOption, optionInfo);
	

	map<double, string, greater<double> > errorRank; // ranking of the absolute value of the relative error of options in each day
	unordered_map<string, double> hc; // number of options to buy or sell during the trading of each day
	unordered_map<string, double> hs; // number of stocks to hedge the corresponding option 
	vector<pair<long double, long double> > capital(12); // the capital of the trader at the start and end of each day

	capital[0]=make_pair(1000000, 1000000);
	capital[1]=capital[0];

	
	// totally 9 valid trading days
	cout<<"The trading begins: "<<endl<<endl<<endl;

	vector<string> date(12);
	date[1]="20121005.txt";
	date[2]="20121008.txt";
	date[3]="20121009.txt";
	date[4]="20121010.txt";
	date[5]="20121011.txt";
	date[6]="20121012.txt";
	date[7]="20121016.txt";
	date[8]="20121017.txt";
	date[9]="20121018.txt";
	date[10]="20121019.txt";

	string outFile;
	ofstream fout;

	for(unsigned int i=1; i<11; i++)
	{

		if(hedge==true)
		{
			outFile="Hedge_"+date[i];
		}
		else
		{
			outFile="naked_"+date[i];
		}

		
		fout.open(outFile);
		fout<<"the trading strategy of the "<<i<<"th day and the result capital is: "<<endl;
		for(unordered_map<string, vector<Option> >::iterator itr=optionInfo.begin(); itr!=optionInfo.end(); itr++)
		{
			errorRank.insert(make_pair(abs(itr->second[i].ticks[0].relError), itr->first));
		}

		for(map<double, string, greater<double> >::iterator itr=errorRank.begin(); itr!=errorRank.end(); itr++)
		{
			if(optionInfo[itr->second][i].ticks[0].absError!=0)
			{
				double hcVolume; // the volume of the options of this kind to buy
				if(optionInfo[itr->second][i].ticks[0].absError<0)
					hcVolume=optionInfo[itr->second][i].ticks[0].lastSize; // the option# to buy
				else if(optionInfo[itr->second][i].ticks[0].absError>0)
					hcVolume=-optionInfo[itr->second][i].ticks[0].lastSize; // the option# to buy
				hc.insert(make_pair(itr->second, hcVolume));
				double hsVolume=-(hcVolume*optionInfo[itr->second][i].ticks[0].delta); // the volume of the hedged stocks
				if(hedge==false)
					hsVolume=0;
				hs.insert(make_pair(itr->second, hsVolume)); // the hedging stocks# to buy
				capital[i].first-=hcVolume*optionInfo[itr->second][i].ticks[0].ask+hsVolume*optionInfo[itr->second][i].ticks[0].stockLast;
				if(capital[i].first<0) // the capital left cannot be negative
				{
					hc.erase(itr->second);
					hs.erase(itr->second);
					capital[i].first+=hcVolume*optionInfo[itr->second][i].ticks[0].ask+hsVolume*optionInfo[itr->second][i].ticks[0].stockLast;
					break;
				}
			}

			capital[i].second=capital[i].first;
			for(unordered_map<string, double>::iterator itr=hc.begin(); itr!=hc.end(); itr++)
			{
				capital[i].second+=hc[itr->first]*optionInfo[itr->first][i].ticks[1].last+hs[itr->first]*optionInfo[itr->first][i].ticks[1].stockLast;
			}
			capital[i+1].first=capital[i].second;
		}

		fout<<"option_instrument	"<<"hc	"<<"hs	"<<"last	"<<"lastSize	"<<"bid	"<<"ask	"<<"position	"<<"stockLast	"<<"stockBid	"<<"stockAsk	"<<"expireDate	"<<"strike	"<<"Type	"<<"interest	"<<"tau	"<<"impVol	"<<"sigma	"<<"delta	"<<"cn_Price	"<<"absError	"<<"relError"<<endl;
		for(unordered_map<string, bool>::iterator itr=activeOption.begin(); itr!=activeOption.end(); itr++)
		{
			Tick* tickTmp0=&optionInfo[itr->first][i].ticks[0];
			Tick* tickTmp1=&optionInfo[itr->first][i].ticks[1];
			if(hc.find(itr->first)!=hc.end()) // this active option is traded
			{
				fout<<itr->first<<"	"<<hc[itr->first]<<"	"<<hs[itr->first]<<"	"<<tickTmp0->last<<"	"<<tickTmp0->lastSize<<"	"<<tickTmp0->bid<<"	"<<tickTmp0->ask<<"	"<<tickTmp0->position<<"	"<<tickTmp0->stockLast<<"	"<<tickTmp0->stockBid<<"	"<<tickTmp0->stockAsk<<"	"<<tickTmp0->expirDate<<"	"<<tickTmp0->strike<<"	"<<tickTmp0->Type<<"	"<<tickTmp0->interest<<"	"<<tickTmp0->tau<<"	"<<tickTmp0->impVol<<"	"<<tickTmp0->sigma<<"	"<<tickTmp0->delta<<"	"<<tickTmp0->cn_Price<<"	"<<tickTmp0->absError<<"	"<<tickTmp0->relError<<endl;
				fout<<itr->first<<"	"<<hc[itr->first]<<"	"<<hs[itr->first]<<"	"<<tickTmp1->last<<"	"<<tickTmp1->lastSize<<"	"<<tickTmp1->bid<<"	"<<tickTmp1->ask<<"	"<<tickTmp1->position<<"	"<<tickTmp1->stockLast<<"	"<<tickTmp1->stockBid<<"	"<<tickTmp1->stockAsk<<"	"<<tickTmp1->expirDate<<"	"<<tickTmp1->strike<<"	"<<tickTmp1->Type<<"	"<<tickTmp1->interest<<"	"<<tickTmp1->tau<<"	"<<tickTmp1->impVol<<"	"<<tickTmp1->sigma<<"	"<<tickTmp1->delta<<"	"<<tickTmp1->cn_Price<<"	"<<tickTmp1->absError<<"	"<<tickTmp1->relError<<endl;
			}
			else
			{
				fout<<itr->first<<"	"<<0<<"	"<<0<<"	"<<tickTmp0->last<<"	"<<tickTmp0->lastSize<<"	"<<tickTmp0->bid<<"	"<<tickTmp0->ask<<"	"<<tickTmp0->position<<"	"<<tickTmp0->stockLast<<"	"<<tickTmp0->stockBid<<"	"<<tickTmp0->stockAsk<<"	"<<tickTmp0->expirDate<<"	"<<tickTmp0->strike<<"	"<<tickTmp0->Type<<"	"<<tickTmp0->interest<<"	"<<tickTmp0->tau<<"	"<<tickTmp0->impVol<<"	"<<tickTmp0->sigma<<"	"<<tickTmp0->delta<<"	"<<tickTmp0->cn_Price<<"	"<<tickTmp0->absError<<"	"<<tickTmp0->relError<<endl;
				fout<<itr->first<<"	"<<0<<"	"<<0<<"	"<<tickTmp1->last<<"	"<<tickTmp1->lastSize<<"	"<<tickTmp1->bid<<"	"<<tickTmp1->ask<<"	"<<tickTmp1->position<<"	"<<tickTmp1->stockLast<<"	"<<tickTmp1->stockBid<<"	"<<tickTmp1->stockAsk<<"	"<<tickTmp1->expirDate<<"	"<<tickTmp1->strike<<"	"<<tickTmp1->Type<<"	"<<tickTmp1->interest<<"	"<<tickTmp1->tau<<"	"<<tickTmp1->impVol<<"	"<<tickTmp1->sigma<<"	"<<tickTmp1->delta<<"	"<<tickTmp1->cn_Price<<"	"<<tickTmp1->absError<<"	"<<tickTmp1->relError<<endl;		
			}
		}
		fout<<"the capital at the end of the trading day is: "<<capital[i].second<<endl<<endl<<endl;

		errorRank.clear();
		hc.clear();
		hs.clear();

		fout.close();
		outFile.clear();
	}

	if(hedge==true)
		outFile="Hedge_capitalSummary";
	else
		outFile="Naked_capitalSummary";
	fout.open(outFile);
	for(unsigned int i=0; i<capital.size(); i++)
	{
		unsigned long long first, second;
		first=capital[i].first;
		second=capital[i].second;
		fout<<"The capital at the end of "<<i<<"th day is "<<second<<endl;
	}
	fout.close();

	system("PAUSE");
	return 0;
}

bool check(const vector<string>& vec, const vector<int>& index)
{
	for(unsigned int i=0; i<index.size(); i++)
	{
		if(vec[index[i]]==" ")
			return false;
	}
	return true;
}

void readFirstDay(const vector<int>& index, const string& fileName, unordered_map<string, bool> &activeOption, unordered_map<string, vector<Option> > & optionInfo)
{
	ifstream in(fileName);
	string line;
	string::size_type lasPos, pos;

	vector<string> parseString;
	vector<Option> emptyVec;
	Tick tickTmp;
	Option emptyOptionTmp;
	string delimiter="	";

	getline(in, line);
	while(getline(in, line)) // read in each line of the file
	{
		lasPos=line.find_first_not_of(delimiter, 0);
		pos=line.find_first_of(delimiter, lasPos);
		while(lasPos!=string::npos || pos!=string::npos)
		{
			parseString.push_back(line.substr(lasPos, pos-lasPos));
			lasPos=line.find_first_not_of(delimiter, pos);
			pos=line.find_first_of(delimiter, lasPos);
		}
		if(check(parseString, index) && parseString[25] == "CALL" && (parseString[10]=="START" || parseString[10]=="END") ) // the tick is complete and valid
		{
			if(activeOption.find(parseString[11])==activeOption.end())
			{
				activeOption.insert(make_pair(parseString[11], false));
				optionInfo.insert(make_pair(parseString[11], emptyVec));
				optionInfo[parseString[11]].push_back(emptyOptionTmp);
			}

			tickTmp.timestamp=parseString[0];
			tickTmp.last=stod(parseString[1]);
			tickTmp.lastSize=stod(parseString[2])*100;
			tickTmp.bid=stod(parseString[4]);
			tickTmp.ask=stod(parseString[5]);
			tickTmp.position=parseString[10];
			tickTmp.symbol=parseString[11];
			tickTmp.stockLast=stod(parseString[13]);
			tickTmp.stockBid=stod(parseString[16]);
			tickTmp.stockAsk=stod(parseString[17]);
			tickTmp.strike=stod(parseString[24]);
			tickTmp.interest=stod(parseString[26])/100;
			tickTmp.tau=stod(parseString[27])/365;
		
			//calculate option statistics
	
			tickTmp.cn_impv();

			optionInfo[tickTmp.symbol].at(0).ticks.push_back(tickTmp);
			
		}
		parseString.clear();
	}
}

void readOtherDay(const vector<int>& index, const string& fileName, unordered_map<string, bool> &activeOption, unordered_map<string, vector<Option> > & optionInfo)
{
	cout<<"haha"<<endl;
	ifstream in(fileName);
	string line;
	string::size_type lasPos, pos;

	vector<string> parseString;
	Tick tickTmp;
	Option emptyOptionTmp;
	string delimiter="	";

	getline(in, line);
	while(getline(in, line)) // read in each line of the file
	{
		lasPos=line.find_first_not_of(delimiter, 0);
		pos=line.find_first_of(delimiter, lasPos);
		while(lasPos!=string::npos || pos!=string::npos)
		{
			parseString.push_back(line.substr(lasPos, pos-lasPos));
			lasPos=line.find_first_not_of(delimiter, pos);
			pos=line.find_first_of(delimiter, lasPos);
		}
		if(check(parseString, index) && parseString[25] == "CALL" && (parseString[10]=="START" || parseString[10]=="END") ) // the tick is complete and valid
		{
			if(activeOption.find(parseString[11])!=activeOption.end()) // the tick belongs to the active option
			{
				if(activeOption[parseString[11]]==false)
				{
					activeOption[parseString[11]]=true;
					optionInfo[parseString[11]].push_back(emptyOptionTmp); // option info on a new day
				}

				tickTmp.timestamp=parseString[0];
				tickTmp.last=stod(parseString[1]);
				tickTmp.lastSize=stod(parseString[2])*100;
				tickTmp.bid=stod(parseString[4]);
				tickTmp.ask=stod(parseString[5]);
				tickTmp.position=parseString[10];
				tickTmp.symbol=parseString[11];
				tickTmp.stockLast=stod(parseString[13]);
				tickTmp.stockBid=stod(parseString[16]);
				tickTmp.stockAsk=stod(parseString[17]);
				tickTmp.strike=stod(parseString[24]);
				tickTmp.interest=stod(parseString[26])/100;
				tickTmp.tau=stod(parseString[27])/365;

				//calculate option statistics
				unsigned int dayNum=optionInfo[tickTmp.symbol].size()-1;
				tickTmp.cn_impv();

				//tickTmp.sigma is computed using the implied vilatility of the end tick of the previous day 
				tickTmp.sigma=optionInfo[tickTmp.symbol].at(dayNum-1).ticks[1].impVol;

				tickTmp.cn_price();

				tickTmp.compute_error();

				optionInfo[tickTmp.symbol].at(dayNum).ticks.push_back(tickTmp);
			}
		}
		parseString.clear();
	}

	//delete the inactive opton from the global data structure 
	vector<string> optionToDelete;
	for(unordered_map<string, bool>::iterator itr=activeOption.begin(); itr!=activeOption.end(); itr++)
	{
		if(itr->second==false)
			optionToDelete.push_back(itr->first);
		else
			itr->second=false; // set to false for further use 
	}
	for(unsigned int i=0; i<optionToDelete.size(); i++)
	{
		activeOption.erase(optionToDelete[i]);
		optionInfo.erase(optionToDelete[i]);
	}

}
