#include "rawdata.h"

#include <iostream>
#include <math.h>
#include <string>
#include "util.h"
#include "init.h"
#include "wallet.h"
#include "main.h"
#include "coins.h"
#include "rpcserver.h"

int onehour = 3600;
	
int oneday = onehour *24;
	
int oneweek = oneday * 7;
	
int onemonth = oneweek *4;
	
int oneyear = onemonth *12;

int Rawdata::totalnumtx()  //total number of chain transactions
{
	int ttnmtx = chainActive.Tip()->nChainTx;

	return ttnmtx;
} 

int Rawdata::incomingtx()  //total number of incoming transactions
{
	int ttnmtx = 0;
	
        for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
        {
            const CWalletTx& wtx = (*it).second;
            if (!wtx.IsTrusted() || wtx.GetBlocksToMaturity() > 0)
                continue;

            CAmount allFee;
            string strSentAccount;
            list<COutputEntry> listReceived;
            list<COutputEntry> listSent;
         return  listSent.size(); 
        }

	return ttnmtx;
} 

int Rawdata::outgoingtx()  //total number of outgoing transactions
{
	int ttnmtx = 0;
	
        for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
        {
            const CWalletTx& wtx = (*it).second;
            if (!wtx.IsTrusted() || wtx.GetBlocksToMaturity() > 0)
                continue;

            CAmount allFee;
            string strSentAccount;
            list<COutputEntry> listReceived;
            list<COutputEntry> listSent;
            wtx.GetAmounts(listReceived, listSent, allFee, strSentAccount, 0);
            
            BOOST_FOREACH(const COutputEntry& s, listSent)
            {
                ttnmtx++;
            return  ttnmtx;
			}
        }
	
	return ttnmtx;
} 

int Rawdata::getNumTransactions() const //number of wallet transactions
{
	//total number of transactions for the account
	int numTransactions = pwalletMain->mapWallet.size();
		return numTransactions; 
}

bool Rawdata::verifynumtx ()
{
	if ((outgoingtx()+ incomingtx())!=getNumTransactions () )
		return false;
}


int Rawdata::netheight ()
{
	return chainActive.Height();
}

CAmount Rawdata::blockreward (int nHeight, CAmount& nFees )
{
	CAmount x = GetBlockValue(nHeight, nFees);	
		return x;
}

CAmount Rawdata::banksubsidy (int nHeight, CAmount& nFees )
{
	CAmount x = GetBanknodePayment(nHeight, nFees);	
		return x;
}

CAmount Rawdata::votesubsidy (int nHeight, CAmount& nFees )
{
	CAmount x = GetBanknodePayment(nHeight, nFees);	
		return x;
}

double Rawdata::networktxpart() //wallet's network participation
{
	double netpart = pwalletMain->mapWallet.size()/chainActive.Tip()->nChainTx;

	return netpart;
	
}

double Rawdata::lifetime() //wallet's lifetime 
{
	int creationdate  = pwalletMain->GetOldestKeyPoolTime();
	int lifespan = (GetTime() - creationdate);
    
	return lifespan;
}

int Rawdata::gbllifetime() //blockchain lifetime in days
{
	int a  = GetTime() - 1418504572;
             
	return a/86400;
}

int64_t Rawdata::balance() 
{
	int64_t bal = pwalletMain->GetBalance()/COIN;

	return bal;
}
	
CAmount Rawdata::Getbankreserve() 
{
	Bidtracker r;
	CAmount reserve = r.btcgetbalance();
	return reserve;
}

CAmount Rawdata::Getbankbalance() 
{
	string bankaddr ="5qoFUCqPUE4pyjus6U6jD6ba4oHR6NZ7c7";
	
	CBitcreditAddress address(bankaddr);
    CTxDestination dest = address.Get();
		
	return Getbankreserve();
}

CAmount Rawdata::Getgrantbalance() 
{
	string grantaddr ="69RAHjiTbn1n6BEo8kPMq6czjZJGg77GbW";
	
	CBitcreditAddress address(grantaddr);
    CTxDestination dest = address.Get();
	
	return Getbankreserve();
}

CAmount Rawdata::Getescrowbalance() 
{
	string escrowaddr ="5qH4yHaaaRuX1qKCZdUHXNJdesssNQcUct";
	
	CBitcreditAddress address(escrowaddr);
    CTxDestination dest = address.Get();
	
	return Getbankreserve();
}

CAmount Rawdata::Getgblmoneysupply()
{
		CCoinsStats ss;
		FlushStateToDisk();
		
		if (pcoinsTip->GetStats(ss)) 
		{
		CAmount x =ss.nTotalAmount/COIN;
		return x;
		}
		return 0;
	
}


CAmount Rawdata::Getgrantstotal()
{
	int blocks = chainActive.Height() - 40000;
	int totalgr = blocks * 10;
	return totalgr;  
}

