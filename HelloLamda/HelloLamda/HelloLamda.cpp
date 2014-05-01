// HelloLamda.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <string>
#include <list>


class Character
{
public:
	Character();
	~Character();

	void Name( const std::string name )
	{
		m_Name = name;
	}

	void SetMoney( unsigned int money )
	{
		m_Money = money;
	}

	std::string GetName() { return m_Name; }
	unsigned int GetMoney() { return m_Money; }

private:
	std::string m_Name;
	unsigned int m_Money;

};

Character::Character()
{
}

Character::~Character()
{
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::list<Character> charList;

	Character alcoholBreak;
	alcoholBreak.Name( "alcoholBreak" );
	alcoholBreak.SetMoney( 1000 );
	charList.push_back( alcoholBreak );

	Character sm7;
	sm7.Name( "sm7" );
	sm7.SetMoney( 1000 );
	charList.push_back( sm7 );

	Character miko;
	miko.Name( "miko" );
	miko.SetMoney( 10000000 );
	charList.push_back( miko );



	Character a;
	a.Name( "aaaa" );
	a.SetMoney( 3 );
	charList.push_back( a );
	Character b;
	b.Name( "bbbb" );
	b.SetMoney( 1 );
	charList.push_back( b );
	Character c;
	c.Name( "cccc" );
	c.SetMoney( 2 );
	charList.push_back( c );
	Character d;
	d.Name( "dddd" );
	d.SetMoney( 6 );
	charList.push_back( d );

	charList.sort( []( Character x, Character y )
	{
		//return x.GetMoney( ) > y.GetMoney( );
		return x.GetName( ) < y.GetName( );
	}
	);

	while ( !charList.empty( ) )
	{
		Character cha = charList.front( );
		printf_s( "Character Nmae:%s \n", cha.GetName().c_str() );
		charList.pop_front( );
	}

	getchar();
	return 0;
}

