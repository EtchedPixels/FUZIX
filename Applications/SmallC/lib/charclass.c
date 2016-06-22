isalpha(c) char c;{
        if ((c >= 'a' & c <= 'z') |
            (c >= 'A' & c <= 'Z'))    return(1);
        else                            return(0);
        }

isupper(c) char c;{
        if (c >= 'A' & c <= 'Z')      return(1);
        else                            return(0);
        }

islower(c) char c;{
        if (c >= 'a' & c <= 'z')      return(1);
        else                            return(0);
        }

isdigit(c) char c;{
        if (c >= '0' & c <= '9')      return(1);
        else                            return(0);
        }

isspace(c) char c;{
        if (c == ' ' | c == '\t' | c == '\n')   return(1);
        else                                    return(0);
        }

toupper(c) char c;{
        return ((c >= 'a' && c <= 'z') ? c - 32: c);
        }

tolower(c) char c;{
        return((c >= 'A' && c <= 'Z') ? c + 32: c);
        }
