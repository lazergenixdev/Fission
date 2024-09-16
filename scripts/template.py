import argparse
from string import Template
from os import path, makedirs

def print_error(message: str):
    print(f'\033[31m{path.basename(__file__)}: {message}\033[0m')

# Returns True on Failure
def generate(file: str, mapping_str: str, out_dir: str | None) -> bool:
    if '.template' not in file:
        print_error(f"Input file ({file!r}) is not a template")
        return True

    try:
        with open(file, 'r') as f:
            template = Template(f.read())
    except Exception as e:
        print_error(e)
        return True

    try:
        mapping = eval(mapping_str)
    except Exception as e:
        print_error('Failed to eval expression')
        return True

    try:
        result = template.substitute(mapping)
    except Exception as e:
        print_error(f'Substitution Failure: {e}')
        return True
    
    try:
        out_file = path.basename(file).replace('.template', '')
        
        if out_dir:
            if not path.isdir(out_dir):
                makedirs(out_dir)
            out_file = path.join(out_dir, out_file)
        with open(out_file, 'w') as f:
            f.write(result)
    except Exception as e:
        print_error(e)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generate file from Template file")
    parser.add_argument('-O', '--output-dir', help='Output file to given directory')
    parser.add_argument('file', help='Input template file')
    parser.add_argument('mappings', help='Template mappings (as Python Dictionary)', default='{}')
    args = parser.parse_args()
    generate(args.file, args.mappings, args.output_dir)