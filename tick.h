#ifndef TICK_H
#define TICK_H

/* Tick.h
 *
 * Representation of a tick
 *
 */

#include <string>

using std::ofstream;
using std::endl;
using std::string;

class Tick {

public:
	string timestamp;
	double last;
	int lastSize;
	double bid;
	double ask;
	string position;
	string symbol;

	double stockLast;
	double stockBid;
	double stockAsk;

	string expirDate;
	double strike;
	string Type;
	double interest;
	double tau;

	double impVol;
	double sigma;
	double delta;
	double cn_Price;
	double absError;
	double relError;

	//EFFECTS: calculates black scholes option price
	//MODIFIES: blsPrice
	void cn_price();

	//EFFECTS: calculates implied volitility
	//MODIFIES: impVol
	void cn_impv();

	//EFFECTS: calculate difference bettween bsPrice and bid/ask.
	//If ask < blsPrice, absError = ask - blsPrice; relError = absError/bsPrice;
	//If bid > blsPrice, absError = bid - blsPrice; relError = absError/bsPrice;
	//MODIFIES: absError, relError
	void compute_error();

};

#endif