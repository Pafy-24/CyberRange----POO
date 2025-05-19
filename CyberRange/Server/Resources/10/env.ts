import { z } from 'zod';

const envSchema = z.object({
  FLAG_PART_1: z.string().default("CTF{fake_part1"),
  FLAG_PART_2: z.string().default("_flagpart2}")
});

const processEnv = {
  FLAG_PART_1: process.env.FLAG_PART_1,
  FLAG_PART_2: process.env.FLAG_PART_2
};

export const env = envSchema.parse(processEnv);