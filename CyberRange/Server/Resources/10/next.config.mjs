/** @type {import('next').NextConfig} */
const nextConfig = {
  images: {
    domains: ['images.unsplash.com', 'api.dicebear.com', 'www.shutterstock.com'],
  },
  async headers() {
    return [
        {
            source: '/:path*',
            headers: [
                { key: 'Cache-Control', value: 'no-store, no-cache, must-revalidate, proxy-revalidate' },
                { key: 'Pragma', value: 'no-cache' },
                { key: 'Expires', value: '0' },
            ],
        },
    ];
  },
};

export default nextConfig;
