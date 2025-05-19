import { NextRequest, NextResponse } from 'next/server';
import { writeFile, access } from 'fs/promises';
import { join, dirname, extname, basename } from 'path';
import { existsSync, mkdirSync } from 'fs';

const uploadDir = join(process.cwd(), 'public', 'uploads');

if (!existsSync(uploadDir)) {
  mkdirSync(uploadDir, { recursive: true });
}
const ALLOWED_FILES = ['.gitignore'];

export async function POST(request: NextRequest) {
  try {    
    const formData = await request.formData();
    const file = formData.get('file') as File;
    const customPath = formData.get('path') as string || '';
    
    if (!file) {
      return NextResponse.json(
        { error: 'No file provided' },
        { status: 400 }
      );
    }
    
    if (!file.name.toLowerCase().endsWith('.mp4')) {
      return NextResponse.json(
        { error: 'Only MP4 files are allowed' },
        { status: 400 }
      );
    }
    
    let filePath;
    let isAllowed = false;

    if (customPath) {
      filePath = join(process.cwd(), customPath);
      const fileName = basename(customPath);
      const isAppPath = filePath.includes('/app/') || filePath.endsWith('/app');
      if (isAppPath) {
        if (fileName === '.gitignore') {
          isAllowed = true;
        } else {
          return NextResponse.json(
            { error: 'Cannot modify existing files' },
            { status: 403 }
          );
        }
      } else {
        if (ALLOWED_FILES.includes(fileName)) {
          isAllowed = true;
        } else {
          try {
            await access(filePath);
            return NextResponse.json(
              { error: 'Cannot modify existing files' },
              { status: 403 }
            );
          } catch {
            isAllowed = true;
          }
        }
      }
      
      const dirPath = dirname(filePath);
      if (!existsSync(dirPath)) {
        try {
          mkdirSync(dirPath, { recursive: true });
        } catch (dirError) {
          console.error('Directory creation error:', dirError);
          return NextResponse.json(
            { error: 'Failed to create directory' },
            { status: 500 }
          );
        }
      }
    } else {
      const filename = `${Date.now()}-${file.name}`;
      filePath = join(uploadDir, filename);
      isAllowed = true;
    }
    
    if (!isAllowed) {
      return NextResponse.json(
        { error: 'File upload not allowed for this path' },
        { status: 403 }
      );
    }
        
    const buffer = Buffer.from(await file.arrayBuffer());
    await writeFile(filePath, buffer);
    
    return NextResponse.json({
      success: true,
      message: 'File uploaded successfully',
      file: {
        name: file.name,
        size: file.size,
        type: file.type,
        path: filePath
      }
    });
    
  } catch (error) {
    console.error('Upload error:', error);
    return NextResponse.json({
      error: 'Upload failed',
      details: error instanceof Error ? error.message : String(error)
    }, {
      status: 500
    });
  }
} 