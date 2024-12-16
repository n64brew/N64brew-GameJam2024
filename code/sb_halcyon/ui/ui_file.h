#ifndef UI_FILE_H
#define UI_FILE_H

#define NUM_FONTS 2
#define NUM_SPRITES 6

#ifdef __cplusplus
extern "C"
{
#endif

    // Base directory for UI assets.
    const char *basePath = "rom:/strawberry_byte/ui/";

    // Arrays for font file names and paths:
    // - uiFontFileName: Array of pointers to store full paths to font files after initialization.
    // - uiFontPaths: Constant array of relative paths for each font, appended to basePath at runtime.
    const char *uiFontFileName[NUM_FONTS];
    const char *uiFontPaths[NUM_FONTS] = {
        "fonts/TitanOne-Regular.font64",
        "fonts/OilOnTheWater-ee5O.font64"};

    // Arrays for button sprite file names and paths.
    const char *uiSpriteButtonFileName[NUM_SPRITES];
    const char *uiSpriteButtonPath[NUM_SPRITES] = {
        "buttons/control_stick.ia8.sprite",
        "buttons/d_pad_triggers.ia8.sprite",
        "buttons/c_buttons0.rgba32.sprite",
        "buttons/c_buttons1.rgba32.sprite",
        "buttons/face_buttons0.rgba32.sprite",
        "buttons/face_buttons1.rgba32.sprite"};

    // Arrays for button sprite file names and paths.
    const char *uiSpritePanelFileName[3];
    const char *uiSpritePanelPath[3] = {
        "panels/gloss.ia4.sprite",
        "panels/pattern_tessalate.ia4.sprite",
        "panels/clouds.ia8.sprite",
    };

    // Arrays for logo sprite file names and paths. See LICENSE.txt for attribution.
    const char *uiSpriteLogoFileName[NUM_SPRITES];
    const char *uiSpriteLogoPath[NUM_SPRITES] = {
        "logos/sb_b0.rgba32.sprite",
        "logos/sb_b1.rgba32.sprite",
        "logos/sb_top.rgba32.sprite",
        "logos/t3d.ia8.sprite",
        "logos/libdragon.ia4.sprite",
        "logos/mixamo.ia4.sprite"};

    /* Declarations */

    char *ui_filePath(const char *fn);
    void ui_fileFonts(void);
    void ui_fileSprites(void);
    void ui_fileLogos(void);
    void ui_fileGet(void);
    void ui_fileCleanup(void);

    /* Definitions */

    // Concatenates basePath and fn, returning the full path (caller must free memory).
    char *ui_filePath(const char *fn)
    {
        char *fullPath = (char *)malloc(256 * sizeof(char));
        if (!fullPath)
        {
            return NULL;
        }

        sprintf(fullPath, "%s%s", basePath, fn);

        return fullPath;
    }

    // Populates uiFontFileName with full paths for fonts.
    void ui_fileFonts(void)
    {
        for (int i = 0; i < NUM_FONTS; ++i)
        {
            uiFontFileName[i] = ui_filePath(uiFontPaths[i]);
        }
    }

    // Populates uiSpriteButtonFileName and uiSpritePanelFileName with full paths for sprites.
    void ui_fileSprites(void)
    {
        for (int i = 0; i < NUM_SPRITES; ++i)
        {
            uiSpriteButtonFileName[i] = ui_filePath(uiSpriteButtonPath[i]);
        }
        for (int i = 0; i < 3; ++i)
        {
            uiSpritePanelFileName[i] = ui_filePath(uiSpritePanelPath[i]);
        }
    }

    void ui_fileLogos(void)
    {
        for (int i = 0; i < NUM_SPRITES; ++i)
        {
            uiSpriteLogoFileName[i] = ui_filePath(uiSpriteLogoPath[i]);
        }
    }

    // Calls functions to initialize font and sprite file paths.
    void ui_fileGet(void)
    {
        ui_fileFonts();
        ui_fileSprites();
        ui_fileLogos();
    }

    // Frees memory allocated for font and sprite file paths.
    void ui_fileCleanup(void)
    {
        for (int i = 0; i < NUM_FONTS; i++)
        {
            free((char *)uiFontFileName[i]);
            ;
        }

        for (int i = 0; i < 3; i++)
        {
            free((char *)uiSpritePanelFileName[i]);
        }

        for (int i = 0; i < NUM_SPRITES; i++)
        {
            free((char *)uiSpriteButtonFileName[i]);
            free((char *)uiSpriteLogoFileName[i]);
        }
    }

#ifdef __cplusplus
}
#endif

#endif // UI_FILE_H