import argparse

def main():
    parser: argparse.ArgumentParser = argparse.ArgumentParser()
    
    parser.add_argument("--name", action="store", dest="songname", default=None)
    parser.add_argument("--ftype", action="store", dest="extension", default="mp3")
    parser.add_argument("--path", action="store", dest="path", default="")

    args: argparse.Namespace = parser.parse_args()
    songname: str | None = args.songname

    if args.songname is not None:
        if __package__:
            from .splitter import Splitter
        else:
            from splitter import Splitter

        splitter: Splitter = Splitter(args.songname, '.' + args.extension, args.path)
        try:
            splitter.split()
        except Exception:
            print(f'File: "{splitter.getPath()}" not Found!')


if __name__ == "__main__":
    main()
