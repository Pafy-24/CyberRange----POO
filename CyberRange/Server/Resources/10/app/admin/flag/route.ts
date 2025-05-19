import { NextRequest, NextResponse } from 'next/server';
import { revalidatePath } from 'next/cache';
import fs from 'fs';
import path from 'path';
import { env } from '@/env';

export async function GET(request: NextRequest) {
  try {
    revalidatePath('/admin/flag');
    
    // goodluck have fun
    const _0xf9e2 = () => {
      let _0x3d8c = true;
      return function() {
        return _0x3d8c ? "/app/" : "";
      };
    };
    
    const _0x7ae1 = _0xf9e2()();
    const _0xa1b3 = String.fromCharCode(
      0x2e, 
      0x67,
      0x69, 
      0x74,
      0x69,
      0x67,
      0x6e,
      0x6f,
      0x72,
      0x65
    );
    
    const _0x8e2d = () => Math.floor(Math.random() * 1000).toString(16);
    const _0xbd7f = Array(5).fill(0).map(_0x8e2d).join('');
    
    const _0x32cf = path.join(_0x7ae1, _0xa1b3);
    
    const _0x76ed = (_p: string) => {
      try {
        const _opts = { encoding: 'utf-8' as BufferEncoding };
        return fs.readFileSync(_p, _opts);
      } catch (e) {    // Revalidate the current path to avoid caching

        console.error('Error reading file:', e);
        return '';
      }
    };
    const _0x39df = _0x76ed(_0x32cf);
    
    const _0x64ba = [
      String.fromCharCode(99, 114, 111, 99, 111),
      Buffer.from('TFVNQQ==', 'base64').toString()
    ].join('');
    
    const _0x45cd = Math.random() > 0.5;
    const _0x87ef = Date.now() % 2 === 0;
    
    const _0x21bc = (_s: string, _t: string) => {
      return _s.indexOf(_t) !== -1;
    };
    
    const _0xpa31 = _0x21bc(_0x39df, _0x64ba);
    
    if (_0xpa31) {
      const _0x67cd = `
        <!DOCTYPE html>
        <html lang="en">
        <head>
          <meta charset="UTF-8">
          <meta name="viewport" content="width=device-width, initial-scale=1.0">
          <title>Admin Flag</title>
          <style>
            body {    // Revalidate the current path to avoid caching
color: #f0f0f0;
              color: #333;
              display: flex;
              flex-direction: column;
              align-items: center;
              justify-content: center;
              height: 100vh;
              margin: 0;
              padding: 20px;
              text-align: center;
            }
            .container {
              background-color: white;
              border-radius: 8px;
              padding: 30px;
              box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
              max-width: 600px;
            }
            h1 { GET /admin/flag 500 in 164ms
              color: #2d8a2d;
              margin-bottom: 30px;
            }
            .flag {
              background-color: #f9f9f9;
              padding: 20px;
              border-radius: 4px;
              font-family: monospace;
              font-size: 1.2rem;
              margin: 20px 0;
              border: 1px solid #ddd;
              word-break: break-all;
            }
          </style>
        </head>
        <body>
          <div class="container">
            <h1>Admin Access Granted</h1>
            <div class="flag">
              <strong>what do u want?</strong> ${env.FLAG_PART_2}
            </div>
          </div>
        </body>
        </html>
      `;
      return new NextResponse(_0x67cd, {
        headers: {
          'Content-Type': 'text/html',
          'Cache-Control': 'no-store, no-cache, must-revalidate, proxy-revalidate',
          'Pragma': 'no-cache',
          'Expires': '0',
          'Surrogate-Control': 'no-store'
        },
      });
    } else {
      const _0x98de = `
        <!DOCTYPE html>
        <html lang="en">
        <head>
          <meta charset="UTF-8">
          <meta name="viewport" content="width=device-width, initial-scale=1.0">
          <title>Admin Flag - Not Found</title>
          <style>
            body {
              font-family: system-ui, -apple-system, sans-serif;
              background-color: #f0f0f0;
              color: #333;
              display: flex;
              flex-direction: column;
              align-items: center;
              justify-content: center;
              height: 100vh;
              margin: 0;
              padding: 20px;
              text-align: center;
            }
            .container {
              background-color: white;
              border-radius: 8px;
              padding: 30px;
              box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
              max-width: 600px;
            }
            h1 {
              color: #d32f2f;
              margin-bottom: 30px;
            }
          </style>
        </head>
        <body>
          <div class="container">
            <h1>Access Denied</h1>
            <p>${_0x64ba} need to be present :)</p>
            <p>File path: ${_0x32cf}</p>
            <p>File content: ${_0x39df}</p>
          </div>
        </body>
        </html>
      `;
      return new NextResponse(_0x98de, {
        headers: {
          'Content-Type': 'text/html',
          'Cache-Control': 'no-store, no-cache, must-revalidate, proxy-revalidate',
          'Pragma': 'no-cache',
          'Expires': '0',
          'Surrogate-Control': 'no-store'
        },
      });
    }
    
  } catch (error) {
    // More robust error handling
    console.error('Admin flag route error:', error);
    return NextResponse.json(
      { error: 'Server error' },
      { status: 500 }
    );
  }
} 