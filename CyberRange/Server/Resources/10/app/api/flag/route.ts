import { NextRequest } from 'next/server';

export async function GET(req: NextRequest): Promise<Response> {
  const html = `
    <!DOCTYPE html>
    <html>
      <head>
        <meta charset="utf-8">
        <title>Congratulations!</title>
        <style>
          body {
            font-family: system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background-color: #f0f9ff;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            margin: 0;
            padding: 20px;
            color: #075985;
            text-align: center;
          }
          h1 {
            font-size: 2rem;
            margin-bottom: 1rem;
          }
          p {
            font-size: 1.2rem;
            margin-bottom: 2rem;
          }
          .flag {
            font-weight: bold;
            color: #0c4a6e;
            background-color: #e0f2fe;
            padding: 8px 16px;
            border-radius: 4px;
            display: inline-block;
            margin-bottom: 2rem;
          }
          .gif-container {
            margin-top: 1rem;
            border-radius: 8px;
            overflow: hidden;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
          }
          img {
            display: block;
            max-width: 100%;
          }
          .auth-buttons {
            display: flex;
            flex-direction: row;
            gap: 16px;
            margin-top: 2rem;
          }
          .signin-btn {
            background-color: #0ea5e9;
            color: white;
            padding: 10px 20px;
            border-radius: 9999px;
            text-decoration: none;
            font-weight: 500;
            transition: background-color 0.2s;
          }
          .signin-btn:hover {
            background-color: #0284c7;
          }
          .signup-btn {
            background-color: transparent;
            color: #000000;
            border: 1px solid #000000;
            padding: 10px 20px;
            border-radius: 9999px;
            text-decoration: none;
            font-weight: 500;
            transition: background-color 0.2s;
          }
          .signup-btn:hover {
            background-color: rgba(0, 0, 0, 0.05);
          }
        </style>
      </head>
      <body>
        <h1>🐊 Congratulations! 🐊</h1>
        <p>First part of Cr0cod1ll3 fl4g is here</p>
        <div class="flag">${process.env.FLAG_PART_1}</div>
        <div class="gif-container">
          <img src="https://media.tenor.com/umwyDKtF2ksAAAAM/excited-crocodile.gif" alt="Excited Crocodile" />
        </div>
      </body>
    </html>
  `;

  return new Response(html, {
    headers: {
      "Content-Type": "text/html",
    },
  });
} 