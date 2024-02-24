/*
 * Audacious: Cross platform multimedia player
 * Copyright (c) 2005-2009 Audacious Team
 *
 * Driver for Game_Music_Emu library. See details at:
 * http://www.slack.net/~ant/libs/
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <libaudcore/audstrings.h>
#include <libaudcore/runtime.h>

#include "Data_Reader.h"
#include "configure.h"
#include "plugin.h"
#include "Music_Emu.h"
#include "Gzip_Reader.h"

static const int fade_threshold = 10 * 1000;
static const int fade_length    = 8 * 1000;

static bool log_err(blargg_err_t err)
{
    if (err)
        AUDERR("%s\n", err);
    return !!err;
}

static void log_warning(Music_Emu * emu)
{
    const char *str = emu->warning();
    if (str)
        AUDWARN("%s\n", str);
}

/* Handles URL parsing, file opening and identification, and file
 * loading. Keeps file header around when loading rest of file to
 * avoid seeking and re-reading.
 */
class ConsoleFileHandler {
public:
    String m_path;            // path without track number specification
    int m_track;             // track number (0 = first track)
    Music_Emu* m_emu;         // set to 0 to take ownership
    gme_type_t m_type;

    // Parses path and identifies file type
    ConsoleFileHandler(const char* path, VFSFile &fd);

    // Creates emulator and returns 0. If this wasn't a music file or
    // emulator couldn't be created, returns 1.
    int load(int sample_rate);

    // Deletes owned emu and closes file
    ~ConsoleFileHandler();

private:
    char m_header[4];
    Vfs_File_Reader vfs_in;
    Std_File_Reader file_in;
};

ConsoleFileHandler::ConsoleFileHandler(const char *path, VFSFile &fd)
{
    m_emu   = nullptr;
    m_type  = 0;
    m_track = -1;

    const char * sub;
    uri_parse (path, nullptr, nullptr, & sub, & m_track);
    m_path = String (str_copy (path, sub - path));

    m_track -= 1;

    // open vfs
    vfs_in.reset(fd);

    // now open gzip_reader on top of vfs
    // if (log_err(file_in.open(&vfs_in)))
    //     return;

    // read and identify header
    // TODO: handle zlib files
    if (!log_err(vfs_in.read(m_header, sizeof(m_header))))
    {
        m_type = gme_identify_extension(gme_identify_header(m_header));
        if (!m_type)
        {
            m_type = gme_identify_extension(m_path);
            if (m_type != gme_gym_type) // only trust file extension for headerless .gym files
                m_type = 0;
        }
    }
}

ConsoleFileHandler::~ConsoleFileHandler()
{
    gme_delete(m_emu);
}

int ConsoleFileHandler::load(int sample_rate)
{
    if (!m_type)
        return 1;

    m_emu = gme_new_emu(m_type, sample_rate);
    if (m_emu == nullptr)
    {
        log_err("Out of memory allocating emulator engine. Fatal error.");
        return 1;
    }

    // combine header with remaining file data
    Remaining_Reader reader(m_header, sizeof(m_header), &vfs_in);
    if (log_err(m_emu->load(reader)))
        return 1;

    // files can be closed now
    // gzip_in.close();
    vfs_in.close();

    log_warning(m_emu);


#if 0
    // load .m3u from same directory( replace/add extension with ".m3u")
    char *m3u_path = g_strdup(m_path);
    char *ext = strrchr(m3u_path, '.');
    if (ext == nullptr)
    {
        ext = g_strdup_printf("%s.m3u", m3u_path);
        g_free(m3u_path);
        m3u_path = ext;
    }

    Vfs_File_Reader m3u;
    if (!m3u.open(m3u_path))
    {
        if (log_err(m_emu->load_m3u(m3u))) // TODO: fail if m3u can't be loaded?
            log_warning(m_emu); // this will log line number of first problem in m3u
    }
#endif

    return 0;
}

static int get_track_length(const track_info_t &info)
{
    int length = info.length;
    if (length <= 0)
        length = info.intro_length + 2 * info.loop_length;

    if (length <= 0)
        length = audcfg.loop_length * 1000;
    else if (length >= fade_threshold)
        length += fade_length;

    return length;
}

static char *skip_spaces(char *i)
{
    while(
        (*i == ' ') ||
        (*i == '\t')
    ){
        i++;
    }
    return i;
}

static char *skip_current_line(char *i)
{
    // assumes windows/unix line endings!
    while(
        (*i != '\n')
    ){
        i++;
    }
    return ++i;
}

static char *word_into_buffer(char *source, char *buffer, size_t max_size)
{
    size_t i = 0;
    while (
        (*source != ' ') &&
        (*source != '\t') &&
        (*source != '\r') &&
        (*source != '\n')
    ){
        if (++i == max_size)
        {
            break;
        }
        *buffer++ = *source++;
    }
    *buffer = '\0';
    return source;
}

static char *into_buffer_until_newline(char *source, char *buffer, size_t max_size)
{
    size_t i = 0;
    // also assumes windows/unix line endings!
    while ((*source != '\n') && (*source != '\r')){
        if (++i == max_size)
        {
            break;
        }
        *buffer++ = *source++;
    }
    *buffer = '\0';
    return source;
}

bool ConsolePlugin::read_tag(const char *filename, VFSFile &file, Tuple &tuple, Index<char> *image)
{
    ConsoleFileHandler fh(filename, file);

    auto set_str = [&tuple](Tuple::Field f, const char *s)
        { if (s[0]) tuple.set_str(f, s); };

    if (!fh.m_type)
        return false;

    // prefer !tags.m3u, if available
    char *tags_m3u_path = strdup(fh.m_path);
    char *ext = strrchr(tags_m3u_path, '/');
    if (ext != nullptr)
    {
        // may overflow
        sprintf(ext, "/!tags.m3u");
    }
    Vfs_File_Reader tags_m3u;
    if (tags_m3u.open(tags_m3u_path))
    {
        AUDWARN("Couldn't find !tags.m3u, falling back on GME built-in.");
        free(tags_m3u_path);

    if (fh.load(gme_info_only))
        return false;

    track_info_t info;
    if (log_err(fh.m_emu->track_info(&info, fh.m_track < 0 ? 0 : fh.m_track)))
        return false;

    

    set_str(Tuple::Artist, info.author);
    set_str(Tuple::Album, info.game);
    set_str(Tuple::Title, info.song);
    set_str(Tuple::Copyright, info.copyright);
    set_str(Tuple::Codec, info.system);
    set_str(Tuple::Comment, info.comment);
    set_str(Tuple::Date, info.date);
    set_str(Tuple::Composer, info.composer);

    if (fh.m_track >= 0)
    {
        tuple.set_int(Tuple::Track, fh.m_track + 1);
        tuple.set_int(Tuple::Subtune, fh.m_track + 1);
        tuple.set_int(Tuple::NumSubtunes, info.track_count);
    }
    else
        tuple.set_subtunes(info.track_count, nullptr);

    tuple.set_int (Tuple::Length, get_track_length (info));
    tuple.set_int (Tuple::Channels, 2);

    } else {
        // parse !tags.m3u here, i want to keep the GME here
        // as pristine as possible. Unless of course, the GME
        // guys take the !tags.m3u thing idk.

        char metatag[10];
        char tag_buf[256];

        tags_m3u.seek(0);
        char *tags_buff = (char *) std::malloc(tags_m3u.size());
        char *into_buf = tags_buff;
        tags_m3u.read_avail(tags_buff, tags_m3u.size());

        int num_subtunes = 0;

        while (*into_buf != '\0')
        {
            into_buf = skip_spaces(into_buf);
            if (*into_buf == '#')
            {
                into_buf = skip_spaces(++into_buf);
                switch (*into_buf++) {
                    case '@': // global tag
                        into_buf = word_into_buffer(into_buf, metatag, 10);
                        into_buf = skip_spaces(into_buf);
                        into_buf = into_buffer_until_newline(into_buf, tag_buf, 256);
                        if (!strcmp_nocase(metatag, "album"))
                        {
                            set_str(Tuple::Album, tag_buf);
                        } 
                        else if (!strcmp_nocase(metatag, "company"))
                        {
                            set_str(Tuple::Copyright, tag_buf);
                            set_str(Tuple::Publisher, tag_buf);
                        } 
                        else if (!strcmp_nocase(metatag, "artist"))
                        {
                            set_str(Tuple::AlbumArtist, tag_buf);
                        } 
                        else if (!strcmp_nocase(metatag, "year"))
                        {
                            int year = atoi(tag_buf);
                            tuple.set_int(Tuple::Year, year);
                            set_str(Tuple::Date, tag_buf);
                        }
                        break;
                    case '%': // local tag
                        into_buf = word_into_buffer(into_buf, metatag, 10);
                        into_buf = skip_spaces(into_buf);
                        into_buf = into_buffer_until_newline(into_buf, tag_buf, 256);
                        if (!strcmp_nocase(metatag, "title"))
                        {
                            // TODO: assumes the subtunes are STRICTLY ordered!
                            // assumes this is defined before the subtune declaration...
                            // I should replace this with a list.
                            if (fh.m_track == num_subtunes)
                                set_str(Tuple::Title, tag_buf);
                        } 
                        else if (!strcmp_nocase(metatag, "subtune"))
                        {
                            int subtune_num = atoi(tag_buf);
                            if (fh.m_track == num_subtunes)
                            {
                                tuple.set_int(Tuple::Track, subtune_num + 1);
                                tuple.set_int(Tuple::Subtune, subtune_num + 1);
                            }
                            num_subtunes++;
                        } 
                        else if (!strcmp_nocase(metatag, "artist"))
                        {
                            // TODO: HACKYYY
                            // assumes this is defined after the subtune declaration...
                            if (fh.m_track == num_subtunes - 1)
                                set_str(Tuple::Artist, tag_buf);
                        } 
                        break;
                    default: // a standard comment, ignore
                        break;
                }
            }
            into_buf = skip_current_line(into_buf);
        }

        free(tags_buff);
        
        tuple.set_int(Tuple::NumSubtunes, num_subtunes);
        tuple.set_subtunes(num_subtunes, nullptr);

    }

    return true;
}

bool ConsolePlugin::play(const char *filename, VFSFile &file)
{
    int length, sample_rate;
    track_info_t info;

    // identify file
    ConsoleFileHandler fh(filename, file);
    if (!fh.m_type)
        return false;

    if (fh.m_track < 0)
        fh.m_track = 0;

    // select sample rate
    sample_rate = 0;
    if (fh.m_type == gme_spc_type)
        sample_rate = 32000;
    if (audcfg.resample)
        sample_rate = audcfg.resample_rate;
    if (sample_rate == 0)
        sample_rate = 44100;

    // create emulator and load file
    if (fh.load(sample_rate))
        return false;

    // stereo echo depth
    gme_set_stereo_depth(fh.m_emu, 1.0 / 100 * audcfg.echo);

    // set equalizer
    if (audcfg.treble || audcfg.bass)
    {
        Music_Emu::equalizer_t eq;

        // bass - logarithmic, 2 to 8194 Hz
        double bass = 1.0 - (audcfg.bass / 200.0 + 0.5);
        eq.bass = (long) (2.0 + pow( 2.0, bass * 13 ));

        // treble - -50 to 0 to +5 dB
        double treble = audcfg.treble / 100.0;
        eq.treble = treble * (treble < 0 ? 50.0 : 5.0);

        fh.m_emu->set_equalizer(eq);
    }

    // get info
    length = -1;
    if (!log_err(fh.m_emu->track_info(&info, fh.m_track)))
    {
        if (fh.m_type == gme_spc_type && audcfg.ignore_spc_length)
            info.length = -1;

        length = get_track_length(info);
        set_stream_bitrate(fh.m_emu->voice_count() * 1000);
    }

    // start track
    if (log_err(fh.m_emu->start_track(fh.m_track)))
        return false;

    log_warning(fh.m_emu);

    open_audio(FMT_S16_NE, sample_rate, 2);

    // set fade time
    if (length <= 0)
        length = audcfg.loop_length * 1000;
    if (length >= fade_threshold + fade_length)
        length -= fade_length / 2;
    fh.m_emu->set_fade(length, fade_length);

    while (!check_stop())
    {
        /* Perform seek, if requested */
        int seek_value = check_seek();
        if (seek_value >= 0)
            fh.m_emu->seek(seek_value);

        /* Fill and play buffer of audio */
        int const buf_size = 1024;
        Music_Emu::sample_t buf[buf_size];

        fh.m_emu->play(buf_size, buf);

        write_audio(buf, sizeof(buf));

        if (fh.m_emu->track_ended())
            break;
    }

    return true;
}
