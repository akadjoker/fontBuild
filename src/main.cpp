

#include "pch.h"


#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct Image 
{
    int width;
    int height;
    int channels;
    unsigned char* data;
};

struct Rectangle 
{
    float x;
    float y;
    float width;
    float height;
    int value;
    int offsetX;
    int offsetY;
    int advanceX;
    Image image;
};




#define FONT_SDF_CHAR_PADDING            4      
#define FONT_SDF_ON_EDGE_VALUE         128      
#define FONT_SDF_PIXEL_DIST_SCALE     64.0f    
#define FONT_BITMAP_ALPHA_THRESHOLD     80      


unsigned char *LoadFileData(const char *fileName, int *dataSize)
{
    unsigned char *data = NULL;
    *dataSize = 0;
    FILE *file = fopen(fileName, "rb");
    if (file != NULL)
    {
   
        fseek(file, 0, SEEK_END);
        int size = ftell(file);     
        fseek(file, 0, SEEK_SET);

        if (size > 0)
        {
            data = (unsigned char *)malloc(size*sizeof(unsigned char));

            if (data != NULL)
            {
                    size_t count = fread(data, sizeof(unsigned char), size, file);
                        
                    *dataSize = (int)count;
                
            }
            
        }


        fclose(file);
    }

    return data;
}

int main()
{

        unsigned char *fileData = NULL;
        int fileSize = 0;

        fileData = LoadFileData("pixantiqua.ttf", &fileSize);
        if (fileData == NULL) return 1;

        int fontSize = 22;
        int codepointCount = 127 ;//95; // 127; // 224; // Number of codepoints in the ranges 32..126 and 160..255







       stbtt_fontinfo fontInfo = { 0 };
       

        if (stbtt_InitFont(&fontInfo, (unsigned char *)fileData, 0))    
        {
            float scaleFactor = stbtt_ScaleForPixelHeight(&fontInfo, (float)fontSize);
            int ascent, descent, lineGap;
            stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

         

           
       
           std::vector<Rectangle> glyphs;

        
    
            for (int i = 0; i < codepointCount; i++)
            {
                int chw = 0, chh = 0;  
                int ch = i + 32 ; 
         
                Rectangle charInfo;
                charInfo.value = ch;
                charInfo.image.channels = 1;

                int index = stbtt_FindGlyphIndex(&fontInfo, ch);

                if (index > 0)
                {

                   
                        //if (ch!=32)
                       //charInfo.image.data = stbtt_GetCodepointSDF(&fontInfo, scaleFactor, ch, FONT_SDF_CHAR_PADDING, FONT_SDF_ON_EDGE_VALUE, FONT_SDF_PIXEL_DIST_SCALE, &chw, &chh, &charInfo.offsetX, &charInfo.offsetY); 
                     charInfo.image.data  = stbtt_GetCodepointBitmap(&fontInfo, scaleFactor, scaleFactor, ch,&chw, &chh,  &charInfo.offsetX, &charInfo.offsetY); 

                        charInfo.image.width = chw;
                        charInfo.image.height = chh;
                        charInfo.width = chw;
                        charInfo.height = chh;
                     
                    

                        stbtt_GetCodepointHMetrics(&fontInfo, ch, &charInfo.advanceX, NULL);
                        charInfo.advanceX = (int)((float)charInfo.advanceX*scaleFactor);
                        charInfo.offsetY  += (int)((float)ascent*scaleFactor);


                       for (int p = 0; p < charInfo.image.width*charInfo.image.height; p++)
                        {
                            if (((unsigned char *)charInfo.image.data)[p] < FONT_BITMAP_ALPHA_THRESHOLD) ((unsigned char *)charInfo.image.data)[p] = 0;
                            else ((unsigned char *)charInfo.image.data)[p] = 220;
                        }


                  
                        glyphs.push_back(charInfo);    
                      
                       



                    
                      // int stride_in_bytes = image.width * image.channels;


                      //  std::string fileName = "export/font" + std::to_string(i) + ".png";

                        //stbi_write_tga(fileName.c_str(), image.width, image.height, image.channels, image.data);

                  //      stbi_write_png(fileName.c_str(), image.width, image.height, image.channels, image.data, stride_in_bytes);
//
                        
        
               

                    

                } else
                {
                    std::cout<<"Error: Failed to get glyph "<<i <<std::endl;
                }
       
                        

            }// forcodepointCount;

        std::ofstream outputFile("font.fnt");
        if (!outputFile.is_open()) 
        {
                std::cerr << "Erro ao abrir o arquivo." << std::endl;
                return 1;
            }

            int totalWidth = 0;
            int maxGlyphWidth = 0;
            int padding = 1;
            for (int i = 0; i <(int) glyphs.size(); i++)
            {
                
                if (glyphs[i].image.width > maxGlyphWidth) maxGlyphWidth = glyphs[i].image.width;
                    totalWidth += glyphs[i].image.width + 4*padding;
            }
            Image atlas;

            float totalArea = totalWidth*fontSize*1.2f;
            float imageMinSize = sqrtf(totalArea);
            int imageSize = (int)powf(2, ceilf(logf(imageMinSize)/logf(2)));

            if (totalArea < ((imageSize*imageSize)/2))
            {
                atlas.width = imageSize;    
                atlas.height = imageSize/2;
            }
            else
            {
                atlas.width = imageSize; 
                atlas.height = imageSize;  
            }

            atlas.channels = 1;
            atlas.data = (unsigned char *)calloc(atlas.width*atlas.height*atlas.channels, sizeof(unsigned char));

            int offsetX = padding;
            int offsetY = padding;


            for (int i = 0; i < (int)glyphs.size(); i++)
            {
                if (offsetX >= (atlas.width - glyphs[i].image.width - 2*padding))
                {
                    offsetX = padding;
                    offsetY += (fontSize + 2*padding);
                    if (offsetY > (atlas.height - fontSize - padding))
                    {
                        for(int j = i + 1; j < (int)glyphs.size()); j++)
                        {
                            std::cout<<"Failed to package character"<< j<<"\n";
                            
                        }
                        break;
                    }
                }

                if (glyphs[i].image.data != NULL)
                {
                    for (int y = 0; y < glyphs[i].image.height; y++)
                    {
                        for (int x = 0; x < glyphs[i].image.width; x++)
                        {
                            ((unsigned char *)atlas.data)[(offsetY + y)*atlas.width + (offsetX + x)] = ((unsigned char *)glyphs[i].image.data)[y*glyphs[i].image.width + x];
                        }
                    }
                }
            
               std::free(glyphs[i].image.data);

                char value = static_cast<char>( i + 32);
                float x = offsetX;
                float y = offsetY;
                float w = glyphs[i].image.width;
                float h = glyphs[i].image.height;
                if (w==0)
                    w = 1;
                if (h==0)
                    h = 1;
                int off_x = glyphs[i].offsetX;
                int off_y = glyphs[i].offsetY;
                outputFile<<"Char="<<"\""<< value <<"\""<< "," << x << "," << y << "," << w << "," << h << "," << off_x << "," << off_y << "\n";

                offsetX += (glyphs[i].image.width + 2*padding);
            }

        unsigned char *dataGrayAlpha = (unsigned char *)malloc(atlas.width*atlas.height*sizeof(unsigned char)*2); // Two channels
        for (int i = 0, k = 0; i < atlas.width*atlas.height; i++, k += 2)
        {
            dataGrayAlpha[k] = 255;
            dataGrayAlpha[k + 1] = ((unsigned char *)atlas.data)[i];
        }

        std::free(atlas.data);
        atlas.data = dataGrayAlpha;
  



         int stride = atlas.width * 2;

         stbi_write_png("font.png", atlas.width, atlas.height, 2,(unsigned char*) atlas.data, stride);
         //stbi_write_tga("font5.tga", atlas.width, atlas.height, 2,(unsigned char*) atlas.data);


         std::free(atlas.data);
        outputFile.close();




        } else 
        {
            std::cout<<"Error: Failed to load font data"<<std::endl;
        }
        


    

return 0;
}

