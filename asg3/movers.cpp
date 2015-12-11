#include <gecode/int.hh>
#include <gecode/search.hh>
#include <gecode/minimodel.hh>
#include <gecode/driver.hh>

using namespace Gecode;
using namespace std;



// int n = 4; //n items
// int m = 4; //m movers
// int maxtime = 80;
// int requiretime[4] = {30, 10, 15, 15};
// int requiremovers[4] = {3, 1, 3, 2};

// int n = 5; //n items
// int m = 5; //m movers
// int maxtime = 80;
// int requiretime[5] = {15, 30, 10, 15, 15};
// int requiremovers[5] = {2, 3, 1, 3, 3};

int n; //n items
int m ; //m movers
int maxtime;
int requiretime[10];
int requiremovers[10];

int solutions = 1;

int begin[10];
int finish[10];
int schedule[10][10];

class SplitBrancher : public Brancher { 
protected: 
	ViewArray<Int::IntView> x; 
	// choice definition 
	class PosVal : public Choice { 
	public: 
		int pos; 
		int val; 
		PosVal(const SplitBrancher& b, int p, int v)
			: Choice(b,2), pos(p), val(v) {} 
		virtual size_t size(void) const { 	
			return sizeof(*this); 
		}
		virtual void archive(Archive& e) const { 
			Choice::archive(e); 
			e << pos << val; 
		} 
	}; 
public: 
	SplitBrancher(Home home, ViewArray<Int::IntView>& x0)
		: Brancher(home), x(x0) {} 
	//posting
	static void post(Home home, ViewArray<Int::IntView>& x) { 
		(void) new (home) SplitBrancher(home,x); 
	} 
	//disposal
	virtual size_t dispose(Space& home) { 
		(void) Brancher::dispose(home); 
		return sizeof(*this); 
	} 
	//choice 
	virtual const Choice*  choice(Space& home) {
		for (int i=0; true; i++)
			if(!x[i].assigned())
			{
                return new PosVal(*this,i, (x[i].max()+ x[i].min())/2 );
			}
		GECODE_NEVER;
		return NULL;
	}
	virtual const Choice*  choice(const Space& home, Archive& e) {
		int pos, val;
		e>>pos>>val;
		return new PosVal(*this, pos, val);
	}
	//copy
	SplitBrancher(Space& home, bool share, SplitBrancher& b) 
		: Brancher(home,share,b) { 
		x.update(home,share,b.x); 
	} 
	virtual Brancher* copy(Space& home, bool share) { 
		return new (home) SplitBrancher(home,share,*this); 
	}
	// status 
	virtual bool status(const Space& home) const { 
		for (int i=0; i<x.size(); i++)
			if(!x[i].assigned())
				return true;
		return false; 
	} 
	// commit 
	virtual ExecStatus commit(Space& home, 
							  const Choice& c, 
							  unsigned int a) {
		const PosVal& pv = static_cast<const PosVal&>(c); 
		int pos=pv.pos, val=pv.val; 
		if (a == 0) 
			return me_failed(x[pos].lq(home,val)) ? ES_FAILED : ES_OK; 
		else 
			return me_failed(x[pos].gr(home,val)) ? ES_FAILED : ES_OK; 
	} 
	// print 
	virtual void print(const Space& home, const Choice& c, unsigned int a, std::ostream& o) const { 
		const PosVal& pv = static_cast<const PosVal&>(c); 
		int pos=pv.pos, val=pv.val; 
		if (a == 0) 
			o << "x[" << pos << "] = " << val; 
		else 
			o << "x[" << pos << "] != " << val; 
	} 
 }; 

void SplitBrancher(Home home, const IntVarArgs& x) { 
	if (home.failed()) 
		return; 
	ViewArray<Int::IntView> y(home,x); 
	SplitBrancher::post(home,y); 
} 

class movers : public Script {
protected:

	IntVarArray start;//n
	IntVarArray duration; //n
	IntVarArray end;//n
	IntVarArray assignment; // n*m
	
	IntVar MaxEndTime;
	IntVar MinBeginTime;
	
	IntVarArray fatigue;
	IntVarArray MaxFatiItem;
	
	IntVarArray MaxItemAssignMover;

	const Options& opt;
public:

	enum{
		MODEL_ONE,
		MODEL_TWO
	};

	movers(const Options& _opt) : 
	Script(opt), opt(_opt), 
	start(*this, n), end(*this, n), duration(*this, n), 
	assignment(*this, n*m,0,1), 
	MaxEndTime(*this, 0, maxtime), MinBeginTime(*this, 0, 0), 
	fatigue(*this, n*m, 0, n-1),
	MaxFatiItem (*this, n, 0, n-1),	MaxItemAssignMover(*this, m, 0, n)	
	{   

		Matrix<IntVarArray> Mat(assignment,n,m); // n rows, m colunms
		Matrix<IntVarArray> fati(fatigue,n,m); // n rows, m colunms
		
		
		IntArgs usage (n, requiremovers);
		
		switch(opt.model()){

			case MODEL_ONE:
				
				
				// domain constraint: start, end, duration, usage
	   			for(int i=0; i < n; ++i)
				{
					int LatestStartTime = maxtime - requiretime[i];
					IntVar temp1(*this, 0, LatestStartTime);
					start[i] = temp1;
					
					int EarliestEndTime = requiretime[i];
					IntVar temp2(*this, requiretime[i], maxtime);
					end[i] = temp2;
					
					std::vector<int> a;
					for (int j=0; j < n; ++j){
						a.push_back(requiretime[i] + j);
					}
							
					duration[i] = IntVar(*this, IntSet(&a[0],a.size()));
				}
				
				// cout << "start time domain: " << start <<endl;
				// cout << "end time domain: " << end <<endl;
				// cout << "duration domain: " << duration <<endl;
				
				
				for (int i = 0; i < n; ++i)
				{
					count(*this, Mat.col(i), 1, IRT_EQ, requiremovers[i]);
				}
				
				
				//calc max fatigue value for each movers
				for(int i=0; i < m; i++) {
					count(*this, Mat.row(i), 1, IRT_EQ, MaxItemAssignMover[i]);
				}
    
				//calc fatigue for every task of every worker
				for(int i=0; i < m; i++) 
				{
					for(int j=0; j < n; j++) 
					{
						rel(*this, fati(j,i) < MaxItemAssignMover[i]);
						IntVar z(*this, 0, 0);
						BoolVar b = expr(*this, Mat(j,i)==0);
						ite(*this, b, z, fati(j,i), fati(j,i));
					}
				}
				

				for(int i=0; i < n; ++i)
				{
					rel(*this, start[i] + duration[i] == end[i]);
				}
				
				cumulative(*this, m, start, duration, end, usage);
				
				
				// one worker can not do two things at same time
				for(int i = 0; i < m; ++i)
				{
					for(int j = 0; j < n-1; ++j)
						for(int k = j+1; k < n; ++k)
						{
							rel(*this, (Mat(j,i)==1 && Mat(k,i)==1) >> (start[j] >= end[k] || start[k] >= end[j]));
						}
				}
				
				// 
				for(int i = 0; i < m; ++i)
				{
					for(int j = 0; j < n-1; ++j)
						for(int k = j+1; k < n; ++k)
						{
							rel(*this, (Mat(j,i)==1 && Mat(k,i)==1 && start[j] < start[k]) >> (fati(j,i) < fati(k,i)));
							rel(*this, (Mat(j,i)==1 && Mat(k,i)==1 && start[j] > start[k]) >> (fati(j,i) > fati(k,i)));
						}
				}
				
				
				for(int i=0; i < n; ++i)
				{
					max( *this, fati.col(i), MaxFatiItem[i] );
				}
				
				for(int i = 0; i < n; ++i)
				{
					rel(*this, duration[i] == requiretime[i] + MaxFatiItem[i]);
				}
				
				
				max(*this, end, MaxEndTime);
				min(*this, start, MinBeginTime);

				branch(*this, assignment, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
				branch(*this, start, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
				branch(*this, duration, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
				branch(*this, end, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
				// SplitBrancher(*this, start);
				// SplitBrancher(*this, duration);
				// SplitBrancher(*this, end);
				branch(*this, fatigue, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
				branch(*this, MaxItemAssignMover, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
				branch(*this, MaxFatiItem, INT_VAR_SIZE_MIN(), INT_VAL_MIN());	
			break;

			case MODEL_TWO:



			break;
		}
	}

  // search support
    movers(bool share, movers& s) : Script(share, s), opt(s.opt) {
		assignment.update(*this, share, s.assignment);
		duration.update(*this, share, s.duration);
		start.update(*this, share, s.start);
		end.update(*this, share, s.end);
		fatigue.update(*this, share, s.fatigue);
		MaxEndTime.update(*this, share, s.MaxEndTime);
		MaxFatiItem.update(*this, share, s.MaxFatiItem);
		MaxItemAssignMover.update(*this, share, s.MaxItemAssignMover);
    }
	virtual Script* copy(bool share) {
		return new movers(share,*this);
	}

  // print solution
	void print(std::ostream& os) const {     
	
		//cout << solutions++ << " " <<MaxEndTime <<endl; 
		
		Matrix<IntVarArray> Mat(assignment,n,m); // n rows, m colunms
		
		for(int i = 0; i < n; ++i) 
		{
			begin[i] = start[i].val();
			finish[i] = end[i].val();
			for(int j =0; j < m; ++j)
			{
				schedule[i][j] = Mat(i,j).val();
			}
		}
    
	}

	virtual void constrain(const Space& _cur){
		const movers& cur = static_cast<const movers&>(_cur);
		int score = cur.MaxEndTime.val();
			
		rel(*this, MaxEndTime < score);
	} 

};

// main function
int main(int argc, char* argv[]) {
	
	
	FILE* fp = fopen("data.in", "r");
	fscanf(fp, "%d %d", &maxtime,&m);
	n = 0;
	char item[50];

	while(fscanf(fp, "%s %d %d", item, &requiretime[n], &requiremovers[n]) !=EOF)
	{
		n++;
	}
	
	fclose (fp);
	
	Options opt("The Moving Problem");
	opt.model(movers::MODEL_ONE, "one", "use the traditional way");
	opt.model(movers::MODEL_TWO, "two", "use TupleSet");
	opt.model(movers::MODEL_ONE);
	opt.solutions(0);
	opt.parse(argc, argv);

	Script::run<movers, BAB, Options>(opt);
	
	
	ofstream fout;
	fout.open("data-1155070976.out");
	for(int i = 0; i < n-1; ++i) 
		{
			fout << begin[i] <<" ";
		}
		fout << begin[n-1] << endl;

	for(int i = 0; i < n-1; ++i) 
		{
			fout << finish[i] <<" ";
		}
		fout << finish[n-1] << endl;	
			
	for(int i = 0; i < n; ++i )	
	{
		for(int j =0; j < m-1; ++j)
		{
			fout<< schedule[i][j] <<" ";
		}
		fout << schedule[i][m-1] << endl;
	}
	fout.close();

	return 0;
}


/*
setenv LD_LIBRARY_PATH <dir>/lib
g++ -I<dir>/include -c <dir>/movers.cpp
g++ -o movers -L<dir>/lib movers.o -lgecodekernel -lgecodesupport -lgecodesearch -lgecodeint -lgecodeminimodel -lgecodedriver -lgecodegist
./movers data.in
*/

