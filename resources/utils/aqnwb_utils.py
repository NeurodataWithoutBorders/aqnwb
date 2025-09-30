import argparse
import generate_spec_files
import schematype_to_aqnwb

def main():
    parser = argparse.ArgumentParser(description="AQNWB utilities")
    subparsers = parser.add_subparsers(dest="command")

    # Sub-parser for generating spec files
    parser_spec = subparsers.add_parser("generate-spec", help="Generate spec files")
    parser_spec.set_defaults(func=generate_spec_files.main)

    # Sub-parser for generating types
    parser_types = subparsers.add_parser("generate-types", help="Generate neurodata types")
    parser_types.set_defaults(func=schematype_to_aqnwb.main)

    args = parser.parse_args()
    if hasattr(args, 'func'):
        args.func()
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
