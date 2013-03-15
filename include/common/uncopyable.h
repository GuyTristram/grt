#ifndef UNCOPYABLE_H
#define UNCOPYABLE_H

class Uncopyable
{
public:
	Uncopyable() {}
private:
	Uncopyable( Uncopyable const &);
	Uncopyable &operator=( Uncopyable const &);
};

#endif // UNCOPYABLE_H

