#include <fs/vfs_tool.h>

static char abs_path[PATH_MAX_LEN + 1];
extern struct process* current;

int good_char(char c)
{
    switch(c)
    {
        //case '²': return 0;
        case '\'': return 0;
        case '{': return 0;
        case '(': return 0;
        case '[': return 0;
        case '-': return 0;
        case '|': return 0;
        case '`': return 0;
        case '\\': return 0;
        case '^': return 0;
        case ')': return 0;
        case ']': return 0;
        //case '°': return 0;
        case '+': return 0;
        case '=': return 0;
        case '}': return 0;
        case '\t': return 0;
        case '*': return 0;
        case ',': return 0;
        case ';': return 0;
        case '?': return 0;
        case ' ': return 0;
        case '<': return 0;
        case '>': return 0;
        default: return 1;
    }
    return 1;
}

int good_path(char* path)
{
    int size = strlen(path);
    int i = 0;

    for(i = 0; i < size && good_char(path[i]); i++);

    return (i == size - 1) ? 1 : 0;
}

int path_clean(char* path)
{
    int path_len = strlen(path);
    int i = 0, j = 0;

    memset(abs_path,0,PATH_MAX_LEN + 1); // Clear previous value of abs_path

    if(!good_path(path) || path_len > PATH_MAX_LEN)
    {
        return 0;
    }

    if(path[0]!='/') // If path is realtive ; convert it in absolute path
    {
        int current_len = strlen(current->cwd);
        strncpy(abs_path,current->cwd,current_len);
        strncpy(abs_path+current_len,path,PATH_MAX_LEN-current_len);
        path_len = strlen(abs_path);

        strncpy(path,abs_path,path_len); // Path has to be too long to support abs_path
    }

    for(i = 0; i < path_len; i++)
	{
		while(path[i] == '/' && i < path_len)
		{
			if(path[i + 1] == '.')
			{
				if(path[i + 2] == '.')
				{
					if(path[i + 3] == '/'
					|| path[i + 3] == '\0')
					{
						if(i)
						{
							for(j = i - 1;
							    j >= 0
						            && path[j] != '/';
							    j--);

							if(path[j] != '/')
							{
								return 0;
							}
						}
						else
						{
							j = i;
						}

						strncpy(path + j,
						        path + i + 3,
							PATH_MAX_LEN - j);
						path_len -= (i + 3 - j);
						i = j;
					}
					else
					{
						i += 3;
					}
				}
				else if(path[i + 2] == '/'
				     || path[i + 2] == '\0')
				{
					strncpy(path + i,
					        path + i + 2,
					        PATH_MAX_LEN - i);
					path_len -= 2;
				}
				else
				{
					i++;
				}
			}
			else if(path[i + 1] == '/' || path[i + 1] == '\0')
			{
				strncpy(path + i,
				        path + i + 1,
				        PATH_MAX_LEN - i);
				path_len--;
			}
			else
			{
				break;
			}
		}
	}

	return 1;
}
