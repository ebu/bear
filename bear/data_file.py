import pkg_resources
import os


def get_path(file_url):
    """Get the real path given a url with forms:

    `file:PATH`: load from file at PATH
    `resource:PATH`: load from package resource PATH
    `PATH`: same as `file:PATH`
    """
    if file_url.startswith("resource:"):
        path = file_url.split(":", 1)[1]

        # try to find the file in a couple of different places
        for package, prefix in ("bear", "data/"), ("visr_bear_data", ""):
            try:
                full_path = pkg_resources.resource_filename(package, prefix + path)
            except ModuleNotFoundError:
                continue

            if os.path.isfile(full_path):
                return full_path
        else:
            raise Exception(f"resource '{path}' not found")
    elif file_url.startswith("file:"):
        return file_url.split(":", 1)[1]
    else:
        return file_url
