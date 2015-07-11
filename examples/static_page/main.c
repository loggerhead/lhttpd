#include <lhttpd.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>


#define FILE_LEN 2

l_http_response_t home_page(l_client_t *client, l_hitem_t *args)
{
    return l_create_response_by_file("static/index.html");
}

l_http_response_t static_handler(l_client_t *client, l_hitem_t *args)
{
    const char *filepath = l_pathcat("static/", l_hget(args, "filepath"));
    l_http_response_t response = l_create_response_by_file(filepath);
    L_FREE(filepath);
    return response;
}

l_http_response_t redirect(l_client_t *client, l_hitem_t *args)
{
    return l_create_redirect_response("/");
}

l_http_response_t random_image(l_client_t *client, l_hitem_t *args)
{
    const char *filepaths[FILE_LEN];

    struct dirent *dir = NULL;
    DIR *d = opendir("static/imgs");
    if (d) {
        int j = 0;
        for (dir = readdir(d); dir; dir = readdir(d))
            if (dir->d_name[0] != '.')
                filepaths[j++] = dir->d_name;
    }

    const char *filepath = l_pathcat("static/imgs", filepaths[rand() % FILE_LEN]);
    l_http_response_t response = l_create_response_by_file(filepath);

    L_FREE(filepath);
    closedir(d);

    return response;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    l_add_route("/", HTTP_GET, home_page);
    l_add_route("/static/<filepath:path>", HTTP_GET, static_handler);
    l_add_route("/static/imgs/<imgpath:path>", HTTP_GET, redirect);
    l_add_route("/random_image", HTTP_GET, random_image);

    l_server_t *server = l_create_server();
    l_start_server(server);
    return 0;
}