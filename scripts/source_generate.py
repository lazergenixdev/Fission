from sys import argv

def generate_banner(
	message: str,
	fill: str = '-',
	left: str | None = '',
	right: str | None = '',
	length: int = 59,
):
	length -= len(left);
	length -= len(message) + 2;
	#print(f'{message!r} {fill!r} {left!r} {right!r}')
	#print(length)
	hw = length // 2
	print(f'{left}{fill*hw} {message} {fill*(length-hw)}{right}')

if __name__ == '__main__':
	generate_banner(argv[1], left='// ')

