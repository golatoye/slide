#include <iostream> 
#include <vector>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
using namespace std;
using namespace boost; 

class Game {
public:


	Game( size_t nCounters ); 

	enum SlotContent {
		Missing,
		Circle,
		Cross
	};

	enum State {
		Playing,
		Won,
		Lost
	};


	void move( size_t  idx ); 

	size_t numSlots() const;
	State getState() const;

	const vector<SlotContent> & getSlots() const ;

protected:
	int distance(size_t  idx ) const;
	vector<SlotContent> slots;
	SlotContent getSlotSelection(size_t idx, bool bExpectMissing) const;
	size_t nCounters; 
	size_t nEmptyPosition;
};

Game::Game( size_t _nCounters )
: nCounters(_nCounters)
{
	if (!nCounters) {
		throw runtime_error( "slot must be greater 0" );
	}

	size_t nSlot = 2 * nCounters + 1 ;
	size_t idx; 
	slots.resize( nSlot );
	 
	for ( idx = 0 ; idx <  nSlot ; ++idx ) {
		SlotContent content = Game::Missing ;

		if ( idx < nCounters ) {
			content  =  Game::Circle; 
		} else if (idx > nCounters ) {
			content  =  Game::Cross; 
		} 

		slots[idx] = content; 
	}
	nEmptyPosition = nCounters;
}

const vector<Game::SlotContent> & Game::getSlots() const {
	return slots;
}

Game::State Game::getState() const
{
	Game::State ret = Game::Playing;

	size_t idx, nSlot = numSlots();
	bool rightPosition = false;
	int minDistance = nSlot;
	
	for ( idx = 0 ; idx <  nSlot ; ++idx ) {

		int dist = distance( idx);
		if ( dist > 0 ) {
			minDistance = min(minDistance, dist);			
		}

		if ( idx < nCounters ) {
			rightPosition = (slots[idx] == Game::Cross );
		} else if (idx > nCounters ) {
			rightPosition = (slots[idx] == Game::Circle );
		} else if (idx == nCounters ) {
			rightPosition = (slots[idx] == Game::Missing );
		} 
	}

	if(rightPosition) {
		ret = Game::Won;
	} else if (minDistance>2) {
		ret = Game::Lost;
	}

	return ret; 
}


size_t Game::numSlots() const
{
	return slots.size();
}

Game::SlotContent Game::getSlotSelection(size_t idx, bool bExpectMissing) const
{
	if (idx <  0  || idx > numSlots()-1  ) {
		throw runtime_error( "invalid slot selection" );
	}

	SlotContent content = slots[idx];
	if ( bExpectMissing && content != Game::Missing  ){
		throw runtime_error( "counter already at position"  );
	}

	if ( !bExpectMissing && content == Game::Missing  ){
		throw runtime_error( "no counter position"  );
	}

	return content; 

} 
int Game::distance(size_t  idx ) const {
	int direction = 1; 
	if (Game::Cross == slots[idx]) {
		direction = -1 ;
	}	
	return (direction * (static_cast<int>(nEmptyPosition) - static_cast<int>(idx)));

}
void Game::move( size_t idx ) {
	
	size_t nMaxMove = 2;
	SlotContent content = getSlotSelection(idx, false);

	int nMove = distance(idx);

	if( nMove == 0 ) {
		throw runtime_error( "no counter position"  );
	} else if ( nMove < 0 ) {
		throw runtime_error( "cant move backwards" );
	} else if ( nMove > 2 ) {
		throw runtime_error( "you can only slide(1) or Jump(2)" );
	}

	slots[idx] = Game::Missing ;	
	slots[nEmptyPosition] = content ;
	nEmptyPosition = idx;

}


class PlayerInterface {
public: 
	virtual  int getSelection( const shared_ptr<const Game> game ) const = 0 ;
};

class UI {

public:
	UI(  shared_ptr<Game> pGame , shared_ptr<PlayerInterface> pPlayer  ) ;
	void play() ;

protected: 
	void show();
	shared_ptr<Game> m_pGame; 
	shared_ptr<PlayerInterface> m_pPlayer; 

};


UI::UI( shared_ptr<Game> pGame ,  shared_ptr<PlayerInterface> pPlayer) 
: m_pGame(pGame)
, m_pPlayer(pPlayer) 
{
	if (!m_pGame.get()) {
		throw runtime_error( "no game provided" );
	}

	if (!m_pPlayer.get()) {
		throw runtime_error( "no player provided" );
	}
}

void UI::show(){
	const vector<Game::SlotContent> &  slots = m_pGame->getSlots();

	vector<Game::SlotContent>::const_iterator it = slots.begin();
	for (; it != slots.end(); ++it) {

		std::string counter;
		Game::SlotContent c = *it;
		if (c == Game::Missing ) {
			cout << " ";
		} else if (c == Game::Cross ) {
			cout << "+";
		} else if (c == Game::Circle ) {
			cout << "o";
		}   
	}
	cout << endl;

}

void UI::play() {

	cout << "playing game" << endl; 
	Game::State state = Game::Playing;

	do {
		show();
		
		cout << "What counter should move : ( -1 to exit) " << endl;
		
		int selection ;
		try {
			selection = m_pPlayer->getSelection( m_pGame);  			
		} catch ( const runtime_error & e ) {
			cerr << " Error " << e.what() << endl;
			continue;
		}

		cout << "You selected " << selection << endl;

 		if (selection == -1 ) {
			cout << "stopping game " << endl; ;
			break;
		}

		try {
			m_pGame->move(selection);
		} catch ( const runtime_error & e ) {
			cerr << "Error: " <<   e.what() << endl; ;
		}

		state = m_pGame->getState();

	}  while ( state == Game::Playing  );

	if (state == Game::Won ) {
		cout << "Game Complete you won" << endl;
		show();

	} else  if (state == Game::Lost ){
		cout << "Game Complete you Lost" << endl;
		show();
	} 
}


class Player : public PlayerInterface {
	 int getSelection( const shared_ptr<const Game> game ) const ; 
};

int Player::getSelection( const shared_ptr<const Game> game )  const {

	int index = 0; 
	cout << "Enter slot [0:" << game->numSlots() -1 << "] : " ;
	cin >>  index;
	if (cin.fail()) 
	{
		cin.clear();
		cin.ignore(10000, '\n');
		throw runtime_error( "Please enter a valid integer" );
    }
	return index;
 }

int main( ) {

	shared_ptr<Game> game ( new Game(3)); 
	shared_ptr<PlayerInterface> player ( new Player()); 
	shared_ptr<UI> ui ( new UI(game, player));    
	ui->play();
	return 0;
}